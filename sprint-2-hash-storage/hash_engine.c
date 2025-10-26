/**
 * @file hash_engine.c
 * @brief Core hash table engine using SipHash with linear probing and
 * tombstones.
 *
 * Overview:
 * - Key/value storage backed by a dynamically-sized bucket array.
 * - Uses SipHash-2-4 (seeded with per-process random keys) to compute bucket
 *   indices; supports linear probing for collision resolution.
 * - Tombstone-based deletion enables correct probe chain traversal during
 *   search operations.
 * - Resizing (grow/shrink) is driven by load factor thresholds.
 * - Raw pointers are stored (no deep copies); callers own key/value memory.
 *
 * Collision Resolution:
 * - Linear probing: on hash collision, probe subsequent buckets until finding
 *   an empty slot or matching key.
 * - Tombstones mark deleted entries (key == NULL, key_len != 0) to prevent
 *   premature search termination.
 * - Resize operations rehash all active entries; tombstones are discarded.
 *
 * Thread-safety:
 * - All public API functions are synchronized with engine->engine_lock.
 * - Concurrent access to the same engine is safe; operations are serialized.
 *
 * Ownership and lifetime:
 * - Keys and values are not copied; only pointers and lengths stored.
 *   Callers must keep memory valid until entry deletion or engine destruction.
 * - Counters (item_count, total_memory) are approximate due to tombstone
 *   overhead and lack of precise shrink tracking.
 *
 * Load Factor Thresholds:
 * - MAX_LOAD_FACTOR (0.75): resize grow when exceeded.
 * - MIN_LOAD_FACTOR (0.2): resize shrink when undercut (if bucket_count >
 *   MIN_BUCKET_COUNT).
 */

/**
 * Per-process SipHash key pair (static module state).
 *
 * These 64-bit keys seed SipHash used by hash_engine_hash() to compute
 * bucket indices. They are initialized in hash_engine_init() via
 * siphash_init_random_key() to provide per-process randomness and defend
 * against hash-flooding attacks. If random initialization fails, weak keys
 * (zero-initialized) will be used and a warning is printed.
 *
 * Thread-safety: Initialized once in hash_engine_init(); concurrent reads
 * are safe after that.
 */
static uint64_t hash_key_0 = 0;
static uint64_t hash_key_1 = 0;

#include "hash_engine.h"
#include "bucket.h"
#include "siphash.h"
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	/* Ensure a known initial state before first resize (mutex not yet
	 * init). */
	engine->hash_buckets = NULL;
	engine->bucket_count = 0;
	engine->item_count = 0;
	engine->total_memory = 0;

	int success = hash_engine_resize(engine, bucket_count);
	if (success != 0) {
		return -1;
	}
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
retry:
	pthread_mutex_lock(&engine->engine_lock);

	int idx = hash_engine_hash(engine, key, key_len);
	int first_tombstone = -1;
	int steps = 0;

	while (steps < engine->bucket_count) {
		struct hash_bucket *b = &engine->hash_buckets[idx];

		if (bucket_is_empty(b)) {
			// Insert into first tombstone seen, otherwise here
			int insert_idx
			    = (first_tombstone != -1) ? first_tombstone : idx;
			struct hash_bucket *dst
			    = &engine->hash_buckets[insert_idx];

			// New insert
			dst->key = key;
			dst->key_len = key_len;
			dst->value = value;
			dst->value_len = value_len;

			engine->item_count++;
			engine->total_memory += (int)(key_len + value_len);
			pthread_mutex_unlock(&engine->engine_lock);
			return 0;
		}

		if (bucket_is_tombstone(b) && first_tombstone == -1)
			first_tombstone = idx;
		else if (!bucket_is_tombstone(b) && b->key_len == key_len
			 && memcmp(b->key, key, key_len) == 0) {
			// Overwrite existing key
			engine->total_memory
			    += (int)((ssize_t)key_len - (ssize_t)b->key_len);
			engine->total_memory += (int)((ssize_t)value_len
						      - (ssize_t)b->value_len);

			b->key = key;
			b->key_len = key_len;
			b->value = value;
			b->value_len = value_len;

			pthread_mutex_unlock(&engine->engine_lock);
			return 0;
		}

		idx = (idx + 1) % engine->bucket_count;
		steps++;
	}

	// Table appears full — grow and retry
	pthread_mutex_unlock(&engine->engine_lock);
	if (hash_engine_resize(engine, engine->bucket_count * 2) != 0)
		return -1;
	goto retry;
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
 * - Performs linear probing to handle collisions and verifies key equality
 * using memcmp().
 * - Returned value pointer is owned by the caller; not copied.
 */
int
hash_get(struct hash_engine *engine, const void *key, size_t key_len,
	 const void **value, size_t *value_len)
{
	pthread_mutex_lock(&engine->engine_lock);

	int idx = hash_engine_hash(engine, key, key_len);
	int steps = 0;

	while (steps < engine->bucket_count) {
		struct hash_bucket *b = &engine->hash_buckets[idx];

		if (bucket_is_empty(b)) {
			// Hard miss (no further cluster)
			pthread_mutex_unlock(&engine->engine_lock);
			return -1;
		}
		if (!bucket_is_tombstone(b) && b->key_len == key_len
		    && memcmp(b->key, key, key_len) == 0) {
			if (value)
				*value = b->value;
			if (value_len)
				*value_len = b->value_len;
			pthread_mutex_unlock(&engine->engine_lock);
			return 0;
		}

		idx = (idx + 1) % engine->bucket_count;
		steps++;
	}

	pthread_mutex_unlock(&engine->engine_lock);
	return -1;
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
	pthread_mutex_lock(&engine->engine_lock);

	int idx = hash_engine_hash(engine, key, key_len);
	int steps = 0;

	while (steps < engine->bucket_count) {
		struct hash_bucket *b = &engine->hash_buckets[idx];

		if (bucket_is_empty(b)) {
			pthread_mutex_unlock(&engine->engine_lock);
			return -1;
		}
		if (!bucket_is_tombstone(b) && b->key_len == key_len
		    && memcmp(b->key, key, key_len) == 0) {
			// Adjust counters before tombstone
			engine->item_count--;
			engine->total_memory
			    -= (int)(b->key_len + b->value_len);
			bucket_make_tombstone(b);
			pthread_mutex_unlock(&engine->engine_lock);
			return 0;
		}

		idx = (idx + 1) % engine->bucket_count;
		steps++;
	}

	pthread_mutex_unlock(&engine->engine_lock);
	return -1;
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
hash_engine_get_stats(struct hash_engine *engine, uint32_t *item_count,
		      uint32_t *bucket_count, uint32_t *memory_usage)
{
	pthread_mutex_lock(&engine->engine_lock);
	if (item_count)
		*item_count = (uint32_t)engine->item_count;
	if (bucket_count)
		*bucket_count = (uint32_t)engine->bucket_count;
	if (memory_usage)
		*memory_usage = (uint32_t)engine->total_memory;
	pthread_mutex_unlock(&engine->engine_lock);
	return 0;
}

/**
 * Compute bucket index for a key using SipHash-2-4.
 *
 * Parameters:
 * - engine: Engine providing bucket_count for modulo mapping.
 * - key: Pointer to key bytes (may be NULL if key_len == 0).
 * - key_len: Length of key in bytes.
 *
 * Returns:
 * - Index in the range [0, engine->bucket_count - 1].
 *
 * Thread-safety:
 * - Read-only w.r.t. engine structure; safe to call concurrently.
 *
 * Notes:
 * - Uses process-global SipHash keys initialized in hash_engine_init().
 * - Behavior is undefined if engine->bucket_count == 0 (not expected in normal
 * use).
 */
int
hash_engine_hash(struct hash_engine *engine, const void *key, size_t key_len)
{
	uint64_t hash = siphash24(key, key_len, hash_key_0, hash_key_1);
	return (int)(hash % (uint64_t)engine->bucket_count);
}

/**
 * Resize the bucket array and rehash existing entries.
 *
 * Parameters:
 * - engine: Engine to mutate.
 * - new_bucket_count: Target number of buckets (> 0).
 *
 * Returns:
 * - 0 on success; -1 on invalid count or allocation failure.
 *
 * Thread-safety:
 * - Rehash path is serialized with engine_lock.
 * - Supports an early initialization path before engine_lock is initialized.
 *
 * Notes:
 * - Recomputes each entry's bucket under the new size and moves pointer fields
 * only.
 * - No collision resolution: on collisions during rehash, later entries
 * overwrite earlier ones, potentially dropping data in this stub
 * implementation.
 * - Does not allocate/copy key/value payloads; ownership remains with the
 * caller.
 * - Counters (item_count, total_memory) are preserved.
 * - Frees the old bucket array after swapping in the new one.
 */
int
hash_engine_resize(struct hash_engine *engine, int new_bucket_count)
{
	struct hash_bucket *new_buckets = NULL, *old_buckets;
	int old_count, i;

	if (new_bucket_count <= 0)
		return -1;

	new_buckets = calloc(new_bucket_count, sizeof(struct hash_bucket));
	if (!new_buckets)
		return -1;

	/* First allocation path: init may call resize before mutex is init. */
	if (!engine->hash_buckets || engine->bucket_count == 0) {
		engine->hash_buckets = new_buckets;
		engine->bucket_count = new_bucket_count;
		return 0;
	}

	pthread_mutex_lock(&engine->engine_lock);

	old_buckets = engine->hash_buckets;
	old_count = engine->bucket_count;

	int target_count = new_bucket_count;

	/* Attempt to rehash into new_buckets; grow and retry if it fills up. */
	for (;;) {
		// Reinsert with linear probing
		int failed = 0;

		// Ensure new_buckets matches target_count and is zeroed
		// (first iteration already allocated; later iterations re-alloc
		// below)
		for (i = 0; i < old_count; i++) {
			struct hash_bucket *ob = &old_buckets[i];
			if (!ob->key || bucket_is_tombstone(ob))
				continue;

			uint64_t h = siphash24(ob->key, ob->key_len, hash_key_0,
					       hash_key_1);
			int idx = (int)(h % (uint64_t)target_count);
			int steps = 0;

			while (steps < target_count) {
				struct hash_bucket *nb = &new_buckets[idx];
				if (bucket_is_empty(nb)) {
					nb->key = ob->key;
					nb->key_len = ob->key_len;
					nb->value = ob->value;
					nb->value_len = ob->value_len;
					break;
				}
				idx = (idx + 1) % target_count;
				steps++;
			}
			if (steps == target_count) {
				failed = 1;
				break;
			}
		}

		if (!failed)
			break;

		// Grow and retry: free current attempt and allocate a bigger
		// table
		free(new_buckets);
		target_count *= 2;
		new_buckets = calloc(target_count, sizeof(struct hash_bucket));
		if (!new_buckets) {
			pthread_mutex_unlock(&engine->engine_lock);
			return -1;
		}
		// Loop to retry
	}

	/* Swap tables after successful rehash. */
	engine->hash_buckets = new_buckets;
	engine->bucket_count = target_count;

	pthread_mutex_unlock(&engine->engine_lock);

	free(old_buckets);
	return 0;
}

/**
 * Determine if the engine needs resizing based on load factors.
 *
 * Parameters:
 * - engine: Target engine.
 *
 * Returns:
 * - Non-zero if resizing is recommended; zero otherwise.
 *
 * Thread-safety:
 * - Safe to call concurrently.
 *
 * Notes:
 * - Uses simple load factor thresholds to suggest resizing.
 */
int
needs_resize(struct hash_engine *engine)
{
	float load_factor
	    = (float)engine->item_count / (float)engine->bucket_count;
	if (load_factor > MAX_LOAD_FACTOR) {
		return 1;
	} else if (load_factor < MIN_LOAD_FACTOR) {
		return 1;
	}
	return 0;
}