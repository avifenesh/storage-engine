#ifndef HASH_ENGINE_H
#define HASH_ENGINE_H

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_LOAD_FACTOR 0.7
#define MIN_LOAD_FACTOR 0.2
// Default, initial, max, and min bucket counts
#define DEFAULT_BUCKET_COUNT 1024
#define INITIAL_BUCKET_COUNT 16
#define MAX_BUCKET_COUNT 1048576
#define MIN_BUCKET_COUNT 16

struct hash_bucket {
	const void *key;
	size_t key_len;
	const void *value;
	size_t value_len;
	struct hash_bucket *next;
};

struct hash_engine {
	struct hash_bucket *hash_buckets;
	int bucket_count;
	pthread_mutex_t engine_lock;
	int item_count;
	int total_memory;
};

int hash_engine_init(struct hash_engine *engine, int bucket_count);

int hash_engine_hash(struct hash_engine *engine, const void *key,
		     size_t key_len);

int hash_engine_resize(struct hash_engine *engine, int new_bucket_count);

int hash_put(struct hash_engine *engine, const void *key, size_t key_len,
	     const void *value, size_t value_len);

int hash_get(struct hash_engine *engine, const void *key, size_t key_len,
	     const void **value, size_t *value_len);

int hash_delete(struct hash_engine *engine, const void *key, size_t key_len);

int hash_engine_destroy(struct hash_engine *engine);

int hash_engine_get_stats(struct hash_engine *engine, uint32_t *item_count,
			  uint32_t *bucket_count, uint32_t *memory_usage);

#endif
