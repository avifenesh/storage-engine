#include "hash_engine.h"
#include "siphash.h"
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Add to hash_engine structure or as globals
static uint64_t hash_key_0 = 0;
static uint64_t hash_key_1 = 0;

// Initialize hash engine with random keys
int
hash_engine_init(struct hash_engine *engine, int bucket_count)
{
	engine->hash_buckets
	    = malloc(bucket_count * sizeof(struct hash_bucket));
	if (engine->hash_buckets == NULL) {
		return -1;
	}
	engine->bucket_count = bucket_count;

	// Initialize random keys for this engine instance
	if (siphash_init_random_key(&hash_key_0, &hash_key_1) != 0) {
		printf("Warning: Using weak random keys\n");
	}

	if (pthread_mutex_init(&engine->engine_lock, NULL) != 0) {
		free(engine->hash_buckets);
		return -1;
	}
	engine->item_count = 0;
	engine->total_memory = 0;
	return 0;
}

// Destroy the hash engine
int
hash_engine_destroy(struct hash_engine *engine)
{
	pthread_mutex_destroy(&engine->engine_lock);
	free(engine->hash_buckets);
	return 0;
}

// Put the key and value into the hash engine
int
hash_put(struct hash_engine *engine, const void *key, size_t key_len,
	 const void *value, size_t value_len)
{
	(void)engine;
	(void)key;
	(void)key_len;
	(void)value;
	(void)value_len;
	int idx = hash_engine_hash(engine, key, key_len);
	pthread_mutex_lock(&engine->engine_lock);
	struct hash_bucket *b = &engine->hash_buckets[idx];
	b->key = key;
	b->key_len = key_len;
	b->value = value;
	b->value_len = value_len;
	engine->item_count++;
	engine->total_memory += key_len + value_len;
	pthread_mutex_unlock(&engine->engine_lock);
	return 0;
}

int
hash_get(struct hash_engine *engine, const void *key, size_t key_len,
	 const void **value, size_t *value_len)
{
	(void)engine;
	(void)key;
	(void)key_len;
	(void)value;
	(void)value_len;
	int idx = hash_engine_hash(engine, key, key_len);
	pthread_mutex_lock(&engine->engine_lock);
	struct hash_bucket *b = &engine->hash_buckets[idx];
	*value = b->value;
	*value_len = b->value_len;
	pthread_mutex_unlock(&engine->engine_lock);
	return 0;
}

// Delete the key from the hash engine
int
hash_delete(struct hash_engine *engine, const void *key, size_t key_len)
{
	(void)engine;
	(void)key;
	(void)key_len;
	int idx = hash_engine_hash(engine, key, key_len);
	pthread_mutex_lock(&engine->engine_lock);
	struct hash_bucket *b = &engine->hash_buckets[idx];
	engine->item_count--;
	engine->total_memory -= key_len;
	b->key = NULL;
	b->key_len = 0;
	b->value = NULL;
	b->value_len = 0;
	pthread_mutex_unlock(&engine->engine_lock);
	return 0;
}

// Get the stats from the hash engine
int
hash_get_stats(struct hash_engine *engine, uint32_t *item_count,
	       uint32_t *bucket_count, uint32_t *memory_usage)
{
	(void)engine;
	if (item_count)
		*item_count = 0;
	if (bucket_count)
		*bucket_count = 0;
	if (memory_usage)
		*memory_usage = 0;
	pthread_mutex_lock(&engine->engine_lock);
	*item_count = engine->item_count;
	*bucket_count = engine->bucket_count;
	*memory_usage = engine->total_memory;
	pthread_mutex_unlock(&engine->engine_lock);
	return 0;
}
