/**
 * @file bucket.c
 */

#include "storage/hash/bucket.h"

int
bucket_is_empty(const struct hash_bucket *bucket)
{
	return bucket->key == NULL && bucket->key_len == 0;
}

int
bucket_is_tombstone(const struct hash_bucket *bucket)
{
	return bucket->key == NULL && bucket->key_len != 0;
}

void
bucket_make_tombstone(struct hash_bucket *bucket)
{
	bucket->key = NULL;
}

