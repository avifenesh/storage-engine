/**
 * Hash engine implementation using SipHash for bucket indexing.
 *
 * Overview:
 * - Provides a minimal key/value hash storage with fixed-size bucket array.
 * - Uses SipHash-derived 64-bit hash to map (key, key_len) to a bucket index.
 * - This userspace stub stores raw pointers without copying; callers own
 * memory.
 *
 * Thread-safety:
 * - The engine uses a single pthread_mutex to serialize mutations and reads of
 *   shared state. Callers may invoke API functions concurrently on the same
 *   engine instance.
 *
 * Ownership and lifetime:
 * - Keys and values are not copied; only pointers and lengths are stored.
 *   Callers must ensure that key/value memory remains valid for the lifetime
 *   of the stored entry (until deletion or engine destruction).
 *
 * Limitations (stub):
 * - No collision handling: each bucket holds a single entry; a new put()
 *   for the same index will overwrite previous contents.
 * - Error handling is minimal; most operations return 0 unconditionally.
 * - Memory accounting is approximate and based on provided lengths.
 */

/**
 * Per-process SipHash key pair.
 *
 * These 64-bit keys seed SipHash used by hash_engine_hash() to compute
 * bucket indices. They are initialized in hash_engine_init() via
 * siphash_init_random_key(). If random initialization fails, weak keys
 * (zero-initialized) will be used and a warning is printed.
 */
static uint64_t hash_key_0 = 0;
static uint64_t hash_key_1 = 0;

#include "hash_engine.h"
#include "siphash.h"
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Initialize a hash_engine with the specified number of buckets.
 *
 * Parameters:
 * - engine: Pointer to an allocated struct hash_engine to initialize.
 * - bucket_count: Number of buckets to allocate (array of struct hash_bucket).
 *
 * Returns:
 * - 0 on success; -1 on allocation or mutex init failure.
 *
 * Notes:
 * - Allocates bucket storage and initializes the engine mutex and counters.
 * - Initializes SipHash keys used for hashing; may fall back to weak keys.
 * - Must be called before any other API and not concurrently with them.
 */
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

/**
 * Destroy a previously initialized hash_engine.
 *
 * Parameters:
 * - engine: Pointer to an initialized struct hash_engine.
 *
 * Returns:
 * - 0 on success.
 *
 * Notes:
 * - Destroys the engine mutex and frees bucket storage.
 * - Does not free any key/value memory owned by the caller.
 * - Not thread-safe with concurrent operations on the same engine.
 */
int
hash_engine_destroy(struct hash_engine *engine)
{
	pthread_mutex_destroy(&engine->engine_lock);
	free(engine->hash_buckets);
	return 0;
}

/**
 * Store a key/value pair in the engine.
 *
 * Parameters:
 * - engine: Target engine.
 * - key, key_len: Pointer and length of the key bytes.
 * - value, value_len: Pointer and length of the value bytes.
 *
 * Returns:
 * - 0 on success.
 *
 * Thread-safety:
 * - Internally synchronized; safe to call concurrently.
 *
 * Notes:
 * - This stub performs no collision resolution; the computed bucket is
 *   overwritten. Pointers are stored directly (no deep copy).
 * - Caller must keep key/value memory valid until deletion or overwrite.
 */
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

/**
 * Retrieve the value for a given key.
 *
 * Parameters:
 * - engine: Target engine.
 * - key, key_len: Pointer and length of the key bytes.
 * - value: Out pointer to receive the stored value pointer.
 * - value_len: Out pointer to receive the stored value length.
 *
 * Returns:
 * - 0 on success.
 *
 * Thread-safety:
 * - Internally synchronized; safe to call concurrently.
 *
 * Notes:
 * - This stub does not verify key equality if collisions occurred; it simply
 *   reads the bucket computed from the provided key.
 * - Returned value pointer is owned by the caller; not copied.
 */
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

/**
 * Delete the entry for a given key.
 *
 * Parameters:
 * - engine: Target engine.
 * - key, key_len: Pointer and length of the key bytes.
 *
 * Returns:
 * - 0 on success.
 *
 * Thread-safety:
 * - Internally synchronized; safe to call concurrently.
 *
 * Notes:
 * - This stub clears the computed bucket without verifying key equality.
 * - Adjusts counters; does not free caller-owned key/value memory.
 */
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

/**
 * Retrieve engine statistics.
 *
 * Parameters:
 * - engine: Target engine.
 * - item_count: Out pointer for number of stored items (may be NULL).
 * - bucket_count: Out pointer for number of buckets (may be NULL).
 * - memory_usage: Out pointer for approximate total bytes (may be NULL).
 *
 * Returns:
 * - 0 on success.
 *
 * Thread-safety:
 * - Internally synchronized; safe to call concurrently.
 *
 * Notes:
 * - If an out pointer is NULL, that field is skipped.
 * - Values reflect current state at the time of locking.
 */
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
