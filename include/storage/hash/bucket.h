/**
 * @file bucket.h
 * @brief Public API for bucket state management in open addressing.
 */

#ifndef BUCKET_H
#define BUCKET_H

#include "storage/hash_engine.h"
#include <stddef.h>

int bucket_is_empty(const struct hash_bucket *bucket);
int bucket_is_tombstone(const struct hash_bucket *bucket);
void bucket_make_tombstone(struct hash_bucket *bucket);

#endif /* BUCKET_H */

