/**
 * @file bucket.c
 */

#include "storage/hash/bucket.h"
#include "utils/futex_mutex_wrapper.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int
bucket_is_empty(struct hash_bucket *bucket)
{
	int result;

	futex_mutex_lock(&bucket->lock_futex);
	result = bucket->key == NULL && bucket->key_len == 0;
	futex_mutex_unlock(&bucket->lock_futex);
	return result;
}

int
bucket_is_tombstone(struct hash_bucket *bucket)
{
	int result;

	futex_mutex_lock(&bucket->lock_futex);
	result = bucket->key == NULL && bucket->key_len != 0;
	futex_mutex_unlock(&bucket->lock_futex);
	return result;
}

int
bucket_make_tombstone_unlocked(struct hash_bucket *bucket)
{
	free((void *)bucket->key);
	free((void *)bucket->value);
	bucket->key = NULL;
	bucket->value = NULL;
	bucket->value_len = 0;
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

	futex_mutex_lock(&bucket->lock_futex);

	key_copy = malloc(key_len);
	if (!key_copy) {
		futex_mutex_unlock(&bucket->lock_futex);
		return -ENOMEM;
	}
	value_copy = malloc(value_len);
	if (!value_copy) {
		free(key_copy);
		futex_mutex_unlock(&bucket->lock_futex);
		return -ENOMEM;
	}

	free((void *)bucket->key);
	free((void *)bucket->value);
	memcpy(key_copy, key, key_len);
	bucket->key = key_copy;
	bucket->key_len = key_len;
	memcpy(value_copy, value, value_len);
	bucket->value = value_copy;
	bucket->value_len = value_len;

	futex_mutex_unlock(&bucket->lock_futex);
	return 0;
}

int
bucket_set_replace_value(struct hash_bucket *bucket, const void *value,
			 size_t value_len)
{
	void *value_copy;
	void *old_value;

	futex_mutex_lock(&bucket->lock_futex);

	value_copy = malloc(value_len);
	if (!value_copy) {
		futex_mutex_unlock(&bucket->lock_futex);
		return -ENOMEM;
	}

	old_value = (void *)bucket->value;
	memcpy(value_copy, value, value_len);
	bucket->value = value_copy;
	bucket->value_len = value_len;
	free(old_value);

	futex_mutex_unlock(&bucket->lock_futex);
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

	futex_mutex_unlock(&bucket->lock_futex);
	return 0;
}
