/**
 * @file bucket.h
 * @brief Public API for bucket state management in open addressing.
 */

#ifndef STORAGE_HASH_BUCKET_H
#define STORAGE_HASH_BUCKET_H

#include <pthread.h>
#include <stddef.h>

struct hash_bucket {
	const void *key;
	size_t key_len;
	const void *value;
	size_t value_len;
	struct hash_bucket *next;
	pthread_rwlock_t rwlock;
};

int bucket_is_empty(const struct hash_bucket *bucket);
int bucket_is_tombstone(const struct hash_bucket *bucket);
void bucket_make_tombstone(struct hash_bucket *bucket);
int bucket_init(struct hash_bucket *bucket);
int bucket_destroy(struct hash_bucket *bucket);
int bucket_set(struct hash_bucket *bucket, const void *key, size_t key_len,
	       const void *value, size_t value_len);
int bucket_set_replace_value(struct hash_bucket *bucket, const void *value,
			     size_t value_len);

#endif /* STORAGE_HASH_BUCKET_H */
