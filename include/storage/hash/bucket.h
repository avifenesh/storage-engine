/**
 * @file bucket.h
 * @brief Public API for bucket state management in open addressing.
 */

#ifndef STORAGE_HASH_BUCKET_H
#define STORAGE_HASH_BUCKET_H

#include "utils/futex_mutex_wrapper.h"
#include <stdatomic.h>
#include <stddef.h>

#define BUCKET_EMPTY 0
#define BUCKET_OCCUPIED 1
#define BUCKET_TOMBSTONE 2

struct hash_bucket {
	_Atomic int state;
	const void *key;
	size_t key_len;
	const void *value;
	size_t value_len;
	futex_mutex_t lock_futex;
};

int bucket_state(struct hash_bucket *bucket);
int bucket_is_empty(struct hash_bucket *bucket);
int bucket_is_tombstone(struct hash_bucket *bucket);
int bucket_make_tombstone(struct hash_bucket *bucket);
int bucket_make_tombstone_unlocked(struct hash_bucket *bucket);
int bucket_init(struct hash_bucket *bucket);
int bucket_destroy(struct hash_bucket *bucket);
int bucket_set(struct hash_bucket *bucket, const void *key, size_t key_len,
	       const void *value, size_t value_len);
int bucket_set_replace_value(struct hash_bucket *bucket, const void *value,
			     size_t value_len, size_t *old_value_len);

#endif /* STORAGE_HASH_BUCKET_H */
