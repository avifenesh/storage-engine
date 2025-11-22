/**
 * @file hash_engine.h
 * @brief Public API for a SipHash-based hash table with linear probing.
 */

#ifndef STORAGE_HASH_ENGINE_H
#define STORAGE_HASH_ENGINE_H

#include "storage/hash/bucket.h"
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_LOAD_FACTOR 0.75
#define MIN_LOAD_FACTOR 0.2

#define DEFAULT_BUCKET_COUNT 1024
#define INITIAL_BUCKET_COUNT 16
#define MAX_BUCKET_COUNT 1048576
#define MIN_BUCKET_COUNT 16

struct hash_engine {
	struct hash_bucket *hash_buckets;
	int bucket_count;
	pthread_rwlock_t engine_lock;
	int item_count;
	int total_memory;
};

int hash_engine_init(struct hash_engine *engine, int bucket_count);
int hash_put(struct hash_engine *engine, const void *key, size_t key_len,
	     const void *value, size_t value_len);
int hash_get(struct hash_engine *engine, const void *key, size_t key_len,
	     const void **value, size_t *value_len);
int hash_delete(struct hash_engine *engine, const void *key, size_t key_len);
int hash_engine_destroy(struct hash_engine *engine);
int hash_engine_get_stats(struct hash_engine *engine, uint32_t *item_count,
			  uint32_t *bucket_count, uint32_t *memory_usage);

#endif /* STORAGE_HASH_ENGINE_H */
