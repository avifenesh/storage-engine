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
static inline int hash_engine_hash(struct hash_engine *engine, const void *key,
				   size_t key_len);
static inline int hash_engine_resize(struct hash_engine *engine,
				     int new_bucket_count);
static inline int rehash_insert(struct hash_bucket *buckets, int bucket_count,
				const void *key, size_t key_len,
				const void *value, size_t value_len);
static inline int keys_equal(const void *k1, size_t l1, const void *k2,
			     size_t l2);
static inline int needs_resize(struct hash_engine *engine);

static inline int
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

	/* Explicitly zero-initialize structure fields */
	engine->hash_buckets = NULL;
	engine->bucket_count = 0;
	engine->item_count = 0;
	engine->total_memory = 0;
	/* Note: engine_lock will be initialized by pthread_rwlock_init */

	if (!is_power_of_two(bucket_count)) {
		int n = 1;
		while (n < bucket_count && n < MAX_BUCKET_COUNT)
			n <<= 1;
		bucket_count = n;
	}

	rc = pthread_rwlock_init(&engine->engine_lock, NULL);
	if (rc != 0)
		return -EAGAIN;

	if (hash_key_0 == 0 && hash_key_1 == 0) {
		if (siphash_init_random_key(&k0, &k1) != 0) {
			/* NOLINT: fprintf is safe here, C11 fprintf_s not
			 * available on Linux */
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
	for (int i = 0; i < bucket_count; i++) {
		if (bucket_init(&engine->hash_buckets[i]) != 0) {
			for (int j = 0; j < i; j++)
				bucket_destroy(&engine->hash_buckets[j]);
			free(engine->hash_buckets);
			return -ENOMEM;
		}
	}
	engine->bucket_count = bucket_count;
	engine->item_count = 0;
	engine->total_memory = 0;
	return 0;
}

static inline int
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
	if (!engine || !key || key_len == 0 || !value || value_len == 0)
		return -EINVAL;

	if (needs_resize(engine)) {
		if (engine->bucket_count >= MAX_BUCKET_COUNT)
			return -ENOSPC;
		if (hash_engine_resize(engine, engine->bucket_count * 2) != 0)
			return -ENOMEM;
	}

	index = hash_engine_hash(engine, key, key_len);
	first_tombstone = -1;
	for (i = 0; i < engine->bucket_count; i++) {
		int idx = (index + i) % engine->bucket_count;
		struct hash_bucket *bucket = &engine->hash_buckets[idx];
		if (bucket_is_empty(bucket)) {
			if (first_tombstone != -1)
				idx = first_tombstone;
			if (bucket_set(&engine->hash_buckets[idx], key, key_len,
				       value, value_len)
			    != 0)
				return -ENOMEM;
			engine->item_count++;
			engine->total_memory += (int)(key_len + value_len);
			return 0;
		}
		if (bucket_is_tombstone(bucket) && first_tombstone == -1) {
			first_tombstone = idx;
		} else if (keys_equal(bucket->key, bucket->key_len, key,
				      key_len)) {
			size_t old_value_len = bucket->value_len;
			if (bucket_set_replace_value(bucket, value, value_len)
			    != 0)
				return -ENOMEM;
			engine->total_memory += (int)(value_len - old_value_len);
			return 0;
		}
	}
	return -ENOSPC;
}

int
hash_delete(struct hash_engine *engine, const void *key, size_t key_len)
{
	int index, i;
	if (!engine || !key || key_len == 0)
		return -EINVAL;

	index = hash_engine_hash(engine, key, key_len);
	for (i = 0; i < engine->bucket_count; i++) {
		struct hash_bucket *bucket
		    = &engine->hash_buckets[(index + i) % engine->bucket_count];
		if (bucket_is_empty(bucket)) {
			return -ENOENT;
		}
		if (!bucket_is_tombstone(bucket)
		    && keys_equal(bucket->key, bucket->key_len, key, key_len)) {
			pthread_rwlock_wrlock(&bucket->rwlock);
			engine->item_count--;
			engine->total_memory
			    -= (int)(bucket->key_len + bucket->value_len);
			bucket_make_tombstone(bucket);
			pthread_rwlock_unlock(&bucket->rwlock);
			if (needs_resize(engine))
				(void)hash_engine_resize(
				    engine, engine->bucket_count / 2);
			return 0;
		}
	}
	return -ENOENT;
}

int
hash_engine_destroy(struct hash_engine *engine)
{
	if (!engine)
		return -EINVAL;
	for (int i = 0; i < engine->bucket_count; i++) {
		bucket_destroy(&engine->hash_buckets[i]);
	}
	free(engine->hash_buckets);
	engine->hash_buckets = NULL;
	engine->bucket_count = 0;
	engine->item_count = 0;
	engine->total_memory = 0;
	pthread_rwlock_destroy(&engine->engine_lock);
	return 0;
}

static inline int
rehash_insert(struct hash_bucket *buckets, int bucket_count, const void *key,
	      size_t key_len, const void *value, size_t value_len)
{
	int index
	    = (int)(siphash(key, key_len, hash_key_0, hash_key_1)
		    % (uint64_t)bucket_count);
	for (int i = 0; i < bucket_count; i++) {
		int idx = (index + i) % bucket_count;
		struct hash_bucket *bucket = &buckets[idx];
		if (bucket_is_empty(bucket) || bucket_is_tombstone(bucket)) {
			int rc = bucket_set(bucket, key, key_len, value, value_len);
			return (rc == 0) ? 0 : -rc;
		}
	}
	return -ENOSPC;
}

static inline int
hash_engine_resize(struct hash_engine *engine, int new_bucket_count)
{
	int old_bucket_count;
	struct hash_bucket *old_buckets;
	struct hash_bucket *new_buckets;
	int new_total_memory = 0;
	int new_item_count = 0;

	pthread_rwlock_wrlock(&engine->engine_lock);
	old_bucket_count = engine->bucket_count;
	old_buckets = engine->hash_buckets;
	if (new_bucket_count < MIN_BUCKET_COUNT
	    || new_bucket_count > MAX_BUCKET_COUNT) {
		pthread_rwlock_unlock(&engine->engine_lock);
		return -EINVAL;
	}
	new_buckets = calloc(new_bucket_count, sizeof(struct hash_bucket));
	if (!new_buckets) {
		pthread_rwlock_unlock(&engine->engine_lock);
		return -ENOMEM;
	}
	for (int i = 0; i < new_bucket_count; i++) {
		if (bucket_init(&new_buckets[i]) != 0) {
			for (int j = 0; j < i; j++)
				bucket_destroy(&new_buckets[j]);
			free(new_buckets);
			pthread_rwlock_unlock(&engine->engine_lock);
			return -ENOMEM;
		}
	}
	for (int i = 0; i < old_bucket_count; i++) {
		struct hash_bucket *bucket = &old_buckets[i];
		if (!bucket_is_empty(bucket) && !bucket_is_tombstone(bucket)) {
			int rc = rehash_insert(new_buckets, new_bucket_count,
					       bucket->key, bucket->key_len,
					       bucket->value, bucket->value_len);
			if (rc != 0) {
				/* Rollback on failure */
				for (int j = 0; j < new_bucket_count; j++) {
					bucket_destroy(&new_buckets[j]);
				}
				free(new_buckets);
				pthread_rwlock_unlock(&engine->engine_lock);
				return (rc > 0) ? -rc : rc;
			}
			new_total_memory
			    += (int)(bucket->key_len + bucket->value_len);
			new_item_count++;
		}
	}
	for (int i = 0; i < old_bucket_count; i++)
		bucket_destroy(&old_buckets[i]);
	free(old_buckets);
	engine->hash_buckets = new_buckets;
	engine->bucket_count = new_bucket_count;
	engine->total_memory = new_total_memory;
	engine->item_count = new_item_count;
	pthread_rwlock_unlock(&engine->engine_lock);
	return 0;
}
