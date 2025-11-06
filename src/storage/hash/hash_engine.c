/**
 * @file hash_engine.c
 * @brief Core hash table engine using SipHash with linear probing and
 * tombstones.
 */

#include "storage/hash_engine.h"
#include "storage/hash/bucket.h"
#include "storage/hash/siphash.h"
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static uint64_t hash_key_0 = 0;
static uint64_t hash_key_1 = 0;

static inline int
is_power_of_two(int x)
{
	return x > 0 && (x & (x - 1)) == 0;
}

int
hash_engine_init(struct hash_engine *engine, int bucket_count)
{
	uint64_t k0 = 0, k1 = 0;
	int rc;

	if (!engine || bucket_count <= 0)
		return -EINVAL;

	memset(engine, 0, sizeof(*engine));
	if (!is_power_of_two(bucket_count)) {
		int n = 1;
		while (n < bucket_count && n < MAX_BUCKET_COUNT)
			n <<= 1;
		bucket_count = n;
	}

	rc = pthread_mutex_init(&engine->engine_lock, NULL);
	if (rc != 0)
		return -EAGAIN;

	if (hash_key_0 == 0 && hash_key_1 == 0) {
		if (siphash_init_random_key(&k0, &k1) != 0) {
			fprintf(stderr, "hash_engine_init: warning: weak "
					"SipHash key used\n");
		}
		hash_key_0 = k0;
		hash_key_1 = k1;
		siphash_set_key(k0, k1);
	}

	engine->hash_buckets = calloc(bucket_count, sizeof(struct hash_bucket));
	if (!engine->hash_buckets)
		return -ENOMEM;

	engine->bucket_count = bucket_count;
	engine->item_count = 0;
	engine->total_memory = 0;
	return 0;
}

int
hash_engine_hash(struct hash_engine *engine, const void *key, size_t key_len)
{
	uint64_t h = siphash(key, key_len, hash_key_0, hash_key_1);
	return (int)(h % (uint64_t)engine->bucket_count);
}

int
hash_engine_get_stats(struct hash_engine *engine, uint32_t *item_count,
		      uint32_t *bucket_count, uint32_t *memory_usage)
{
	if (!engine)
		return -EINVAL;
	if (item_count)
		*item_count = (uint32_t)engine->item_count;
	if (bucket_count)
		*bucket_count = (uint32_t)engine->bucket_count;
	if (memory_usage)
		*memory_usage = (uint32_t)engine->total_memory;
	return 0;
}

static inline int
keys_equal(const void *k1, size_t l1, const void *k2, size_t l2)
{
	return (l1 == l2) && (memcmp(k1, k2, l1) == 0);
}

int
hash_get(struct hash_engine *engine, const void *key, size_t key_len,
	 const void **value, size_t *value_len)
{
	int index, i;
	if (!engine || !key || key_len == 0)
		return -EINVAL;

	index = hash_engine_hash(engine, key, key_len);
	for (i = 0; i < engine->bucket_count; i++) {
		struct hash_bucket *bucket
		    = &engine->hash_buckets[(index + i) % engine->bucket_count];
		if (bucket_is_empty(bucket))
			return -ENOENT;
		if (!bucket_is_tombstone(bucket)
		    && keys_equal(bucket->key, bucket->key_len, key, key_len)) {
			if (value)
				*value = bucket->value;
			if (value_len)
				*value_len = bucket->value_len;
			return 0;
		}
	}
	return -ENOENT;
}

int
hash_put(struct hash_engine *engine, const void *key, size_t key_len,
	 const void *value, size_t value_len)
{
	int index, i, first_tombstone;
	if (!engine || !key || key_len == 0)
		return -EINVAL;

	pthread_mutex_lock(&engine->engine_lock);
	if (needs_resize(engine)) {
		pthread_mutex_unlock(&engine->engine_lock);
		if (engine->bucket_count >= MAX_BUCKET_COUNT)
			return -ENOSPC;
		if (hash_engine_resize(engine, engine->bucket_count * 2) != 0)
			return -ENOMEM;
		pthread_mutex_lock(&engine->engine_lock);
	}

	index = hash_engine_hash(engine, key, key_len);
	first_tombstone = -1;
	for (i = 0; i < engine->bucket_count; i++) {
		int idx = (index + i) % engine->bucket_count;
		struct hash_bucket *bucket = &engine->hash_buckets[idx];
		if (bucket_is_empty(bucket)) {
			int target
			    = (first_tombstone != -1) ? first_tombstone : idx;
			struct hash_bucket *dest
			    = &engine->hash_buckets[target];
			dest->key = key;
			dest->key_len = key_len;
			dest->value = value;
			dest->value_len = value_len;
			engine->item_count++;
			engine->total_memory += (int)(key_len + value_len);
			pthread_mutex_unlock(&engine->engine_lock);
			return 0;
		}
		if (bucket_is_tombstone(bucket) && first_tombstone == -1)
			first_tombstone = idx;
		else if (!bucket_is_tombstone(bucket)
			 && keys_equal(bucket->key, bucket->key_len, key,
				       key_len)) {
			engine->total_memory -= (int)bucket->value_len;
			bucket->value = value;
			bucket->value_len = value_len;
			engine->total_memory += (int)value_len;
			pthread_mutex_unlock(&engine->engine_lock);
			return 0;
		}
	}
	pthread_mutex_unlock(&engine->engine_lock);
	return -ENOSPC;
}

int
hash_delete(struct hash_engine *engine, const void *key, size_t key_len)
{
	int index, i;
	if (!engine || !key || key_len == 0)
		return -EINVAL;

	pthread_mutex_lock(&engine->engine_lock);
	index = hash_engine_hash(engine, key, key_len);
	for (i = 0; i < engine->bucket_count; i++) {
		struct hash_bucket *bucket
		    = &engine->hash_buckets[(index + i) % engine->bucket_count];
		if (bucket_is_empty(bucket)) {
			pthread_mutex_unlock(&engine->engine_lock);
			return -ENOENT;
		}
		if (!bucket_is_tombstone(bucket)
		    && keys_equal(bucket->key, bucket->key_len, key, key_len)) {
			engine->item_count--;
			engine->total_memory
			    -= (int)(bucket->key_len + bucket->value_len);
			bucket_make_tombstone(bucket);
			pthread_mutex_unlock(&engine->engine_lock);
			if (needs_resize(engine))
				(void)hash_engine_resize(
				    engine, engine->bucket_count / 2);
			return 0;
		}
	}
	pthread_mutex_unlock(&engine->engine_lock);
	return -ENOENT;
}

int
hash_engine_destroy(struct hash_engine *engine)
{
	if (!engine)
		return -EINVAL;
	free(engine->hash_buckets);
	engine->hash_buckets = NULL;
	engine->bucket_count = 0;
	engine->item_count = 0;
	engine->total_memory = 0;
	return 0;
}

int
hash_engine_resize(struct hash_engine *engine, int new_bucket_count)
{
	struct hash_bucket *new_buckets = NULL, *old_buckets;
	int old_count, i;
	int target_count;

	if (new_bucket_count <= 0)
		return -EINVAL;

	new_buckets = calloc(new_bucket_count, sizeof(struct hash_bucket));
	if (!new_buckets)
		return -ENOMEM;

	if (!engine->hash_buckets || engine->bucket_count == 0) {
		engine->hash_buckets = new_buckets;
		engine->bucket_count = new_bucket_count;
		return 0;
	}

	pthread_mutex_lock(&engine->engine_lock);

	old_buckets = engine->hash_buckets;
	old_count = engine->bucket_count;
	target_count = new_bucket_count;

	for (;;) {
		int failed = 0;
		int j;
		memset(new_buckets, 0,
		       (size_t)target_count * sizeof(struct hash_bucket));
		for (j = 0; j < old_count; j++) {
			struct hash_bucket *ob = &old_buckets[j];
			uint64_t h;
			int idx;
			int steps = 0;

			if (!ob->key || bucket_is_tombstone(ob))
				continue;

			h = siphash(ob->key, ob->key_len, hash_key_0,
				    hash_key_1);
			idx = (int)(h % (uint64_t)target_count);

			while (steps < target_count) {
				struct hash_bucket *nb = &new_buckets[idx];
				if (bucket_is_empty(nb)) {
					nb->key = ob->key;
					nb->key_len = ob->key_len;
					nb->value = ob->value;
					nb->value_len = ob->value_len;
					break;
				}
				idx = (idx + 1) % target_count;
				steps++;
			}
			if (steps == target_count) {
				failed = 1;
				break;
			}
		}

		if (!failed)
			break;

		free(new_buckets);
		target_count *= 2;
		new_buckets = calloc(target_count, sizeof(struct hash_bucket));
		if (!new_buckets) {
			pthread_mutex_unlock(&engine->engine_lock);
			return -ENOMEM;
		}
	}

	engine->hash_buckets = new_buckets;
	engine->bucket_count = target_count;

	pthread_mutex_unlock(&engine->engine_lock);

	free(old_buckets);
	return 0;
}

int
needs_resize(struct hash_engine *engine)
{
	float load_factor
	    = (float)engine->item_count / (float)engine->bucket_count;

	if (load_factor > MAX_LOAD_FACTOR
	    && engine->bucket_count < MAX_BUCKET_COUNT)
		return 1;

	if (load_factor < MIN_LOAD_FACTOR
	    && engine->bucket_count > MIN_BUCKET_COUNT)
		return 1;

	return 0;
}
