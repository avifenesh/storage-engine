/**
 * @file hash_engine.c
 * @brief Core hash table engine using SipHash with linear probing and
 * tombstones.
 */

#include "storage/hash_engine.h"
#include "storage/hash/bucket.h"
#include "storage/hash/siphash.h"
#include <errno.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static _Atomic int siphash_initialized = 0;
static uint64_t hash_key_0 = 0;
static uint64_t hash_key_1 = 0;
static futex_mutex_t siphash_init_lock;

static inline uint32_t compute_hash(const void *key, size_t key_len,
				    uint32_t bucket_count);
static inline int keys_equal(const void *k1, size_t l1, const void *k2,
			     size_t l2);
static void migrate_bucket(struct hash_engine *engine,
			   struct hash_bucket *old_bucket);
static void migrate_some_buckets(struct hash_engine *engine, uint32_t count);
static void finish_resize(struct hash_engine *engine);
static int hash_engine_start_resize(struct hash_engine *engine,
				    uint32_t new_bucket_count);

static inline int
needs_grow(struct hash_engine *engine)
{
	uint32_t count = atomic_load(&engine->item_count);
	uint32_t buckets = atomic_load(&engine->bucket_count);
	return count >= buckets * MAX_LOAD_FACTOR;
}

static inline int
needs_shrink(struct hash_engine *engine)
{
	uint32_t count = atomic_load(&engine->item_count);
	uint32_t buckets = atomic_load(&engine->bucket_count);
	return buckets > MIN_BUCKET_COUNT && count < buckets * MIN_LOAD_FACTOR;
}

static inline uint32_t
compute_hash(const void *key, size_t key_len, uint32_t bucket_count)
{
	uint64_t h = siphash(key, key_len, hash_key_0, hash_key_1);
	return (uint32_t)(h % bucket_count);
}

static inline int
keys_equal(const void *k1, size_t l1, const void *k2, size_t l2)
{
	return (l1 == l2) && (memcmp(k1, k2, l1) == 0);
}

static void
init_siphash_keys(void)
{
	uint64_t k0 = 0, k1 = 0;

	if (atomic_load(&siphash_initialized))
		return;

	futex_mutex_lock(&siphash_init_lock);
	if (atomic_load(&siphash_initialized)) {
		futex_mutex_unlock(&siphash_init_lock);
		return;
	}

	if (siphash_init_random_key(&k0, &k1) != 0) {
		fprintf(stderr, "hash_engine_init: warning: weak "
				"SipHash key used\n");
	}
	hash_key_0 = k0;
	hash_key_1 = k1;
	siphash_set_key(k0, k1);
	atomic_store(&siphash_initialized, 1);
	futex_mutex_unlock(&siphash_init_lock);
}

int
hash_engine_init(struct hash_engine *engine, uint32_t bucket_count)
{
	struct hash_bucket *buckets;

	if (!engine || bucket_count == 0)
		return -EINVAL;

	futex_mutex_init(&engine->engine_lock);
	atomic_store(&engine->hash_buckets, NULL);
	atomic_store(&engine->bucket_count, 0);
	atomic_store(&engine->item_count, 0);
	atomic_store(&engine->total_memory, 0);
	atomic_store(&engine->old_buckets, NULL);
	atomic_store(&engine->old_bucket_count, 0);
	atomic_store(&engine->migrate_index, 0);
	atomic_store(&engine->migrate_workers, 0);

	init_siphash_keys();

	buckets = calloc(bucket_count, sizeof(struct hash_bucket));
	if (!buckets)
		return -ENOMEM;

	for (uint32_t i = 0; i < bucket_count; i++) {
		if (bucket_init(&buckets[i]) != 0) {
			for (uint32_t j = 0; j < i; j++)
				bucket_destroy(&buckets[j]);
			free(buckets);
			return -ENOMEM;
		}
	}

	atomic_store(&engine->hash_buckets, buckets);
	atomic_store(&engine->bucket_count, bucket_count);
	return 0;
}

int
hash_engine_get_stats(struct hash_engine *engine, uint32_t *item_count,
		      uint32_t *bucket_count, uint32_t *memory_usage)
{
	if (!engine)
		return -EINVAL;
	if (item_count)
		*item_count = atomic_load(&engine->item_count);
	if (bucket_count)
		*bucket_count = atomic_load(&engine->bucket_count);
	if (memory_usage)
		*memory_usage = atomic_load(&engine->total_memory);
	return 0;
}

static int
lookup_in_table(struct hash_bucket *buckets, uint32_t bucket_count,
		const void *key, size_t key_len, const void **value,
		size_t *value_len)
{
	uint32_t index = compute_hash(key, key_len, bucket_count);

	for (uint32_t i = 0; i < bucket_count; i++) {
		struct hash_bucket *bucket
		    = &buckets[(index + i) % bucket_count];
		int state = bucket_state(bucket);

		if (state == BUCKET_EMPTY)
			return -ENOENT;
		if (state == BUCKET_TOMBSTONE)
			continue;

		futex_mutex_lock(&bucket->lock_futex);
		if (atomic_load(&bucket->state) != BUCKET_OCCUPIED) {
			futex_mutex_unlock(&bucket->lock_futex);
			continue;
		}
		if (keys_equal(bucket->key, bucket->key_len, key, key_len)) {
			if (value)
				*value = bucket->value;
			if (value_len)
				*value_len = bucket->value_len;
			futex_mutex_unlock(&bucket->lock_futex);
			return 0;
		}
		futex_mutex_unlock(&bucket->lock_futex);
	}
	return -ENOENT;
}

static int
insert_into_table(struct hash_bucket *buckets, uint32_t bucket_count,
		  const void *key, size_t key_len, const void *value,
		  size_t value_len, int *is_new, size_t *old_value_len)
{
	uint32_t index = compute_hash(key, key_len, bucket_count);
	int tombstone_idx = -1;

	for (uint32_t i = 0; i < bucket_count; i++) {
		uint32_t idx = (index + i) % bucket_count;
		struct hash_bucket *bucket = &buckets[idx];
		int state = bucket_state(bucket);

		if (state == BUCKET_EMPTY) {
			uint32_t target_idx = (tombstone_idx >= 0)
						  ? (uint32_t)tombstone_idx
						  : idx;
			struct hash_bucket *target = &buckets[target_idx];
			void *key_copy;
			void *value_copy;

			futex_mutex_lock(&target->lock_futex);
			state = atomic_load(&target->state);
			if (state != BUCKET_EMPTY
			    && state != BUCKET_TOMBSTONE) {
				futex_mutex_unlock(&target->lock_futex);
				continue;
			}
			free((void *)target->key);
			free((void *)target->value);

			key_copy = malloc(key_len);
			if (!key_copy) {
				futex_mutex_unlock(&target->lock_futex);
				return -ENOMEM;
			}
			value_copy = malloc(value_len);
			if (!value_copy) {
				free(key_copy);
				futex_mutex_unlock(&target->lock_futex);
				return -ENOMEM;
			}
			memcpy(key_copy, key, key_len);
			memcpy(value_copy, value, value_len);
			target->key = key_copy;
			target->key_len = key_len;
			target->value = value_copy;
			target->value_len = value_len;
			atomic_store(&target->state, BUCKET_OCCUPIED);
			futex_mutex_unlock(&target->lock_futex);

			if (is_new)
				*is_new = 1;
			return 0;
		}

		if (state == BUCKET_TOMBSTONE) {
			if (tombstone_idx < 0)
				tombstone_idx = (int)idx;
			continue;
		}

		futex_mutex_lock(&bucket->lock_futex);
		if (atomic_load(&bucket->state) != BUCKET_OCCUPIED) {
			futex_mutex_unlock(&bucket->lock_futex);
			continue;
		}
		if (keys_equal(bucket->key, bucket->key_len, key, key_len)) {
			void *value_copy = malloc(value_len);
			if (!value_copy) {
				futex_mutex_unlock(&bucket->lock_futex);
				return -ENOMEM;
			}
			memcpy(value_copy, value, value_len);
			if (old_value_len)
				*old_value_len = bucket->value_len;
			free((void *)bucket->value);
			bucket->value = value_copy;
			bucket->value_len = value_len;
			futex_mutex_unlock(&bucket->lock_futex);
			if (is_new)
				*is_new = 0;
			return 0;
		}
		futex_mutex_unlock(&bucket->lock_futex);
	}

	if (tombstone_idx >= 0) {
		struct hash_bucket *target = &buckets[tombstone_idx];
		int tstate;
		void *key_copy;
		void *value_copy;

		futex_mutex_lock(&target->lock_futex);
		tstate = atomic_load(&target->state);
		if (tstate != BUCKET_TOMBSTONE) {
			futex_mutex_unlock(&target->lock_futex);
			return -ENOSPC;
		}
		free((void *)target->key);
		free((void *)target->value);

		key_copy = malloc(key_len);
		if (!key_copy) {
			futex_mutex_unlock(&target->lock_futex);
			return -ENOMEM;
		}
		value_copy = malloc(value_len);
		if (!value_copy) {
			free(key_copy);
			futex_mutex_unlock(&target->lock_futex);
			return -ENOMEM;
		}
		memcpy(key_copy, key, key_len);
		memcpy(value_copy, value, value_len);
		target->key = key_copy;
		target->key_len = key_len;
		target->value = value_copy;
		target->value_len = value_len;
		atomic_store(&target->state, BUCKET_OCCUPIED);
		futex_mutex_unlock(&target->lock_futex);

		if (is_new)
			*is_new = 1;
		return 0;
	}

	return -ENOSPC;
}

static int
delete_from_table(struct hash_bucket *buckets, uint32_t bucket_count,
		  const void *key, size_t key_len, size_t *deleted_key_len,
		  size_t *deleted_value_len)
{
	uint32_t index = compute_hash(key, key_len, bucket_count);

	for (uint32_t i = 0; i < bucket_count; i++) {
		struct hash_bucket *bucket
		    = &buckets[(index + i) % bucket_count];
		int state = bucket_state(bucket);

		if (state == BUCKET_EMPTY)
			return -ENOENT;
		if (state == BUCKET_TOMBSTONE)
			continue;

		futex_mutex_lock(&bucket->lock_futex);
		if (atomic_load(&bucket->state) != BUCKET_OCCUPIED) {
			futex_mutex_unlock(&bucket->lock_futex);
			continue;
		}
		if (keys_equal(bucket->key, bucket->key_len, key, key_len)) {
			if (deleted_key_len)
				*deleted_key_len = bucket->key_len;
			if (deleted_value_len)
				*deleted_value_len = bucket->value_len;
			bucket_make_tombstone_unlocked(bucket);
			futex_mutex_unlock(&bucket->lock_futex);
			return 0;
		}
		futex_mutex_unlock(&bucket->lock_futex);
	}
	return -ENOENT;
}

static void
migrate_bucket(struct hash_engine *engine, struct hash_bucket *old_bucket)
{
	struct hash_bucket *new_buckets;
	uint32_t new_bucket_count;
	int state = bucket_state(old_bucket);

	if (state != BUCKET_OCCUPIED)
		return;

	new_buckets = atomic_load(&engine->hash_buckets);
	new_bucket_count = atomic_load(&engine->bucket_count);

	futex_mutex_lock(&old_bucket->lock_futex);
	if (atomic_load(&old_bucket->state) != BUCKET_OCCUPIED) {
		futex_mutex_unlock(&old_bucket->lock_futex);
		return;
	}

	insert_into_table(new_buckets, new_bucket_count, old_bucket->key,
			  old_bucket->key_len, old_bucket->value,
			  old_bucket->value_len, NULL, NULL);

	bucket_make_tombstone_unlocked(old_bucket);
	futex_mutex_unlock(&old_bucket->lock_futex);
}

static void
migrate_some_buckets(struct hash_engine *engine, uint32_t count)
{
	struct hash_bucket *old;
	uint32_t old_count;
	uint32_t idx;
	uint32_t i;

	atomic_fetch_add(&engine->migrate_workers, 1);

	old = atomic_load(&engine->old_buckets);
	if (!old) {
		atomic_fetch_sub(&engine->migrate_workers, 1);
		return;
	}

	old_count = atomic_load(&engine->old_bucket_count);

	for (i = 0; i < count; i++) {
		idx = atomic_fetch_add(&engine->migrate_index, 1);
		if (idx >= old_count) {
			atomic_fetch_sub(&engine->migrate_workers, 1);
			finish_resize(engine);
			return;
		}
		migrate_bucket(engine, &old[idx]);
	}

	atomic_fetch_sub(&engine->migrate_workers, 1);
}

static void
finish_resize(struct hash_engine *engine)
{
	struct hash_bucket *old;
	uint32_t old_count;
	uint32_t workers;
	uint32_t i;

	if (futex_mutex_trylock(&engine->engine_lock) != 0)
		return;

	old = atomic_load(&engine->old_buckets);
	if (!old) {
		futex_mutex_unlock(&engine->engine_lock);
		return;
	}

	workers = atomic_load(&engine->migrate_workers);
	if (workers > 0) {
		futex_mutex_unlock(&engine->engine_lock);
		return;
	}

	old_count = atomic_load(&engine->old_bucket_count);

	for (i = 0; i < old_count; i++)
		bucket_destroy(&old[i]);
	free(old);

	atomic_store(&engine->old_buckets, NULL);
	atomic_store(&engine->old_bucket_count, 0);
	atomic_store(&engine->migrate_index, 0);

	futex_mutex_unlock(&engine->engine_lock);
}

static int
hash_engine_start_resize(struct hash_engine *engine, uint32_t new_bucket_count)
{
	struct hash_bucket *new_buckets;
	struct hash_bucket *current_buckets;
	uint32_t current_count;

	futex_mutex_lock(&engine->engine_lock);

	if (atomic_load(&engine->old_buckets) != NULL) {
		futex_mutex_unlock(&engine->engine_lock);
		return 0;
	}

	if (new_bucket_count < MIN_BUCKET_COUNT
	    || new_bucket_count > MAX_BUCKET_COUNT) {
		futex_mutex_unlock(&engine->engine_lock);
		return -EINVAL;
	}

	current_count = atomic_load(&engine->bucket_count);
	if (new_bucket_count > current_count) {
		if (!needs_grow(engine)) {
			futex_mutex_unlock(&engine->engine_lock);
			return 0;
		}
	} else {
		if (!needs_shrink(engine)) {
			futex_mutex_unlock(&engine->engine_lock);
			return 0;
		}
	}

	new_buckets = calloc(new_bucket_count, sizeof(struct hash_bucket));
	if (!new_buckets) {
		futex_mutex_unlock(&engine->engine_lock);
		return -ENOMEM;
	}

	for (uint32_t i = 0; i < new_bucket_count; i++) {
		if (bucket_init(&new_buckets[i]) != 0) {
			for (uint32_t j = 0; j < i; j++)
				bucket_destroy(&new_buckets[j]);
			free(new_buckets);
			futex_mutex_unlock(&engine->engine_lock);
			return -ENOMEM;
		}
	}

	current_buckets = atomic_load(&engine->hash_buckets);
	current_count = atomic_load(&engine->bucket_count);

	atomic_store(&engine->old_buckets, current_buckets);
	atomic_store(&engine->old_bucket_count, current_count);
	atomic_store(&engine->migrate_index, 0);
	atomic_store(&engine->hash_buckets, new_buckets);
	atomic_store(&engine->bucket_count, new_bucket_count);

	futex_mutex_unlock(&engine->engine_lock);
	return 0;
}

int
hash_get(struct hash_engine *engine, const void *key, size_t key_len,
	 const void **value, size_t *value_len)
{
	struct hash_bucket *buckets;
	struct hash_bucket *old;
	uint32_t bucket_count;
	int rc;

	if (!engine || !key || key_len == 0)
		return -EINVAL;

	migrate_some_buckets(engine, MIGRATE_BATCH_SIZE);

	buckets = atomic_load(&engine->hash_buckets);
	bucket_count = atomic_load(&engine->bucket_count);
	rc = lookup_in_table(buckets, bucket_count, key, key_len, value,
			     value_len);
	if (rc == 0)
		return 0;

	old = atomic_load(&engine->old_buckets);
	if (old) {
		uint32_t old_count = atomic_load(&engine->old_bucket_count);
		rc = lookup_in_table(old, old_count, key, key_len, value,
				     value_len);
	}

	return rc;
}

int
hash_put(struct hash_engine *engine, const void *key, size_t key_len,
	 const void *value, size_t value_len)
{
	struct hash_bucket *buckets;
	struct hash_bucket *old;
	uint32_t bucket_count;
	int is_new = 0;
	int existed_in_old = 0;
	size_t old_tbl_key_len = 0;
	size_t old_tbl_value_len = 0;
	size_t new_tbl_old_value_len = 0;
	int rc;

	if (!engine || !key || key_len == 0 || !value || value_len == 0)
		return -EINVAL;

	migrate_some_buckets(engine, MIGRATE_BATCH_SIZE);

	if (needs_grow(engine)) {
		uint32_t current = atomic_load(&engine->bucket_count);
		uint32_t new_size = current * 2;
		if (new_size <= MAX_BUCKET_COUNT)
			hash_engine_start_resize(engine, new_size);
	}

	old = atomic_load(&engine->old_buckets);
	if (old) {
		uint32_t old_count = atomic_load(&engine->old_bucket_count);
		if (delete_from_table(old, old_count, key, key_len,
				      &old_tbl_key_len, &old_tbl_value_len)
		    == 0)
			existed_in_old = 1;
	}

	buckets = atomic_load(&engine->hash_buckets);
	bucket_count = atomic_load(&engine->bucket_count);

	rc = insert_into_table(buckets, bucket_count, key, key_len, value,
			       value_len, &is_new, &new_tbl_old_value_len);
	if (rc != 0)
		return rc;

	if (is_new && !existed_in_old) {
		atomic_fetch_add(&engine->item_count, 1);
		atomic_fetch_add(&engine->total_memory,
				 (uint32_t)(key_len + value_len));
	} else if (existed_in_old) {
		atomic_fetch_sub(
		    &engine->total_memory,
		    (uint32_t)(old_tbl_key_len + old_tbl_value_len));
		atomic_fetch_add(&engine->total_memory,
				 (uint32_t)(key_len + value_len));
	} else {
		if (value_len > new_tbl_old_value_len)
			atomic_fetch_add(
			    &engine->total_memory,
			    (uint32_t)(value_len - new_tbl_old_value_len));
		else if (new_tbl_old_value_len > value_len)
			atomic_fetch_sub(
			    &engine->total_memory,
			    (uint32_t)(new_tbl_old_value_len - value_len));
	}

	return 0;
}

int
hash_delete(struct hash_engine *engine, const void *key, size_t key_len)
{
	struct hash_bucket *buckets;
	struct hash_bucket *old;
	uint32_t bucket_count;
	size_t del_key_len = 0;
	size_t del_value_len = 0;
	size_t old_del_key_len = 0;
	size_t old_del_value_len = 0;
	int rc;
	int deleted_from_old = 0;
	int deleted_from_new = 0;

	if (!engine || !key || key_len == 0)
		return -EINVAL;

	migrate_some_buckets(engine, MIGRATE_BATCH_SIZE);

	old = atomic_load(&engine->old_buckets);
	if (old) {
		uint32_t old_count = atomic_load(&engine->old_bucket_count);
		if (delete_from_table(old, old_count, key, key_len,
				      &old_del_key_len, &old_del_value_len)
		    == 0)
			deleted_from_old = 1;
	}

	buckets = atomic_load(&engine->hash_buckets);
	bucket_count = atomic_load(&engine->bucket_count);
	rc = delete_from_table(buckets, bucket_count, key, key_len,
			       &del_key_len, &del_value_len);
	if (rc == 0)
		deleted_from_new = 1;

	if (deleted_from_new || deleted_from_old) {
		atomic_fetch_sub(&engine->item_count, 1);
		if (deleted_from_new)
			atomic_fetch_sub(
			    &engine->total_memory,
			    (uint32_t)(del_key_len + del_value_len));
		else
			atomic_fetch_sub(
			    &engine->total_memory,
			    (uint32_t)(old_del_key_len + old_del_value_len));

		if (needs_shrink(engine)) {
			uint32_t current = atomic_load(&engine->bucket_count);
			uint32_t new_size = current / 2;
			if (new_size >= MIN_BUCKET_COUNT)
				hash_engine_start_resize(engine, new_size);
		}
		return 0;
	}

	return rc;
}

int
hash_engine_destroy(struct hash_engine *engine)
{
	struct hash_bucket *buckets;
	struct hash_bucket *old;
	uint32_t bucket_count;
	uint32_t old_count;

	if (!engine)
		return -EINVAL;

	futex_mutex_lock(&engine->engine_lock);

	buckets = atomic_load(&engine->hash_buckets);
	bucket_count = atomic_load(&engine->bucket_count);
	if (buckets) {
		for (uint32_t i = 0; i < bucket_count; i++)
			bucket_destroy(&buckets[i]);
		free(buckets);
	}

	old = atomic_load(&engine->old_buckets);
	old_count = atomic_load(&engine->old_bucket_count);
	if (old) {
		for (uint32_t i = 0; i < old_count; i++)
			bucket_destroy(&old[i]);
		free(old);
	}

	atomic_store(&engine->hash_buckets, NULL);
	atomic_store(&engine->bucket_count, 0);
	atomic_store(&engine->item_count, 0);
	atomic_store(&engine->total_memory, 0);
	atomic_store(&engine->old_buckets, NULL);
	atomic_store(&engine->old_bucket_count, 0);
	atomic_store(&engine->migrate_index, 0);

	futex_mutex_unlock(&engine->engine_lock);
	return 0;
}
