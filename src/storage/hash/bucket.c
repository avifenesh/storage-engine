/**
 * @file bucket.c
 */

#include "storage/hash/bucket.h"
#include "utils/futex_mutex_wrapper.h"
#include <errno.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

int
bucket_state(struct hash_bucket *bucket)
{
	return atomic_load(&bucket->state);
}

int
bucket_is_empty(struct hash_bucket *bucket)
{
	return atomic_load(&bucket->state) == BUCKET_EMPTY;
}

int
bucket_is_tombstone(struct hash_bucket *bucket)
{
	return atomic_load(&bucket->state) == BUCKET_TOMBSTONE;
}

int
bucket_make_tombstone_unlocked(struct hash_bucket *bucket)
{
	free((void *)bucket->key);
	free((void *)bucket->value);
	bucket->key = NULL;
	bucket->key_len = 0;
	bucket->value = NULL;
	bucket->value_len = 0;
	atomic_store(&bucket->state, BUCKET_TOMBSTONE);
	return 0;
}

int
bucket_make_tombstone(struct hash_bucket *bucket)
{
	futex_mutex_lock(&bucket->lock_futex);
	bucket_make_tombstone_unlocked(bucket);
	futex_mutex_unlock(&bucket->lock_futex);
	return 0;
}

int
bucket_init(struct hash_bucket *bucket)
{
	atomic_store(&bucket->state, BUCKET_EMPTY);
	bucket->key = NULL;
	bucket->key_len = 0;
	bucket->value = NULL;
	bucket->value_len = 0;
	futex_mutex_init(&bucket->lock_futex);
	return 0;
}

int
bucket_set(struct hash_bucket *bucket, const void *key, size_t key_len,
	   const void *value, size_t value_len)
{
	void *key_copy;
	void *value_copy;

	key_copy = malloc(key_len);
	if (!key_copy)
		return -ENOMEM;
	value_copy = malloc(value_len);
	if (!value_copy) {
		free(key_copy);
		return -ENOMEM;
	}
	memcpy(key_copy, key, key_len);
	memcpy(value_copy, value, value_len);

	futex_mutex_lock(&bucket->lock_futex);
	free((void *)bucket->key);
	free((void *)bucket->value);
	bucket->key = key_copy;
	bucket->key_len = key_len;
	bucket->value = value_copy;
	bucket->value_len = value_len;
	atomic_store(&bucket->state, BUCKET_OCCUPIED);
	futex_mutex_unlock(&bucket->lock_futex);
	return 0;
}

int
bucket_set_replace_value(struct hash_bucket *bucket, const void *value,
			 size_t value_len, size_t *old_value_len)
{
	void *value_copy;
	void *old_value;

	value_copy = malloc(value_len);
	if (!value_copy)
		return -ENOMEM;
	memcpy(value_copy, value, value_len);

	futex_mutex_lock(&bucket->lock_futex);
	if (old_value_len)
		*old_value_len = bucket->value_len;
	old_value = (void *)bucket->value;
	bucket->value = value_copy;
	bucket->value_len = value_len;
	futex_mutex_unlock(&bucket->lock_futex);

	free(old_value);
	return 0;
}

int
bucket_destroy(struct hash_bucket *bucket)
{
	futex_mutex_lock(&bucket->lock_futex);
	free((void *)bucket->key);
	free((void *)bucket->value);
	bucket->key = NULL;
	bucket->key_len = 0;
	bucket->value = NULL;
	bucket->value_len = 0;
	atomic_store(&bucket->state, BUCKET_EMPTY);
	futex_mutex_unlock(&bucket->lock_futex);
	return 0;
}
