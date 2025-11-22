/**
 * @file bucket.c
 */

#include "storage/hash/bucket.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int
bucket_is_empty(const struct hash_bucket *bucket)
{
	int result;

	pthread_rwlock_rdlock((pthread_rwlock_t *)&bucket->rwlock);
	result = bucket->key == NULL && bucket->key_len == 0;
	pthread_rwlock_unlock((pthread_rwlock_t *)&bucket->rwlock);
	return result;
}

int
bucket_is_tombstone(const struct hash_bucket *bucket)
{
	int result;

	pthread_rwlock_rdlock((pthread_rwlock_t *)&bucket->rwlock);
	result = bucket->key == NULL && bucket->key_len != 0;
	pthread_rwlock_unlock((pthread_rwlock_t *)&bucket->rwlock);
	return result;
}

void
bucket_make_tombstone(struct hash_bucket *bucket)
{
	free((void *)bucket->key);
	free((void *)bucket->value);
	bucket->key = NULL;
	bucket->value = NULL;
	bucket->value_len = 0;
}

int
bucket_init(struct hash_bucket *bucket)
{
	if (!bucket)
		return EINVAL;
	bucket->key = NULL;
	bucket->key_len = 0;
	bucket->value = NULL;
	bucket->value_len = 0;
	bucket->next = NULL;
	if (pthread_rwlock_init(&bucket->rwlock, NULL) != 0)
		return ENOMEM;
	return 0;
}

int
bucket_destroy(struct hash_bucket *bucket)
{
	void *key_ptr;
	void *value_ptr;

	if (!bucket)
		return EINVAL;
	pthread_rwlock_wrlock(&bucket->rwlock);
	key_ptr = (void *)bucket->key;
	value_ptr = (void *)bucket->value;
	free(key_ptr);
	free(value_ptr);
	bucket->key = NULL;
	bucket->value = NULL;
	bucket->key_len = 0;
	bucket->value_len = 0;
	pthread_rwlock_unlock(&bucket->rwlock);
	pthread_rwlock_destroy(&bucket->rwlock);
	return 0;
}

int
bucket_set(struct hash_bucket *bucket, const void *key, size_t key_len,
	   const void *value, size_t value_len)
{
	void *key_copy;
	void *value_copy;

	if (!bucket)
		return EINVAL;
	if (!key || key_len == 0 || !value || value_len == 0)
		return EINVAL;

	key_copy = malloc(key_len);
	value_copy = malloc(value_len);
	if (!key_copy || !value_copy) {
		free(key_copy);
		free(value_copy);
		return ENOMEM;
	}
	memcpy(key_copy, key, key_len);
	memcpy(value_copy, value, value_len);

	pthread_rwlock_wrlock(&bucket->rwlock);
	free((void *)bucket->key);
	free((void *)bucket->value);
	bucket->key = key_copy;
	bucket->key_len = key_len;
	bucket->value = value_copy;
	bucket->value_len = value_len;
	pthread_rwlock_unlock(&bucket->rwlock);
	return 0;
}

int
bucket_set_replace_value(struct hash_bucket *bucket, const void *value,
			 size_t value_len)
{
	void *value_copy;

	if (!bucket)
		return EINVAL;
	if (!value || value_len == 0)
		return EINVAL;

	value_copy = malloc(value_len);
	if (!value_copy)
		return ENOMEM;
	memcpy(value_copy, value, value_len);

	pthread_rwlock_wrlock(&bucket->rwlock);
	free((void *)bucket->value);
	bucket->value = value_copy;
	bucket->value_len = value_len;
	pthread_rwlock_unlock(&bucket->rwlock);
	return 0;
}
