/**
 * @file hash_engine.c
 * @brief Core hash table engine using SipHash with linear probing and
 * tombstones.
 *
 * OVERVIEW
 * ========
 * This module implements a thread-safe, dynamically-resizing hash table engine
 * designed for high-performance key-value storage. The implementation uses:
 * - SipHash-2-4 cryptographic hash function for defense against hash-flooding
 * - Linear probing for collision resolution
 * - Tombstone-based deletion to maintain probe chain integrity
 * - Automatic resizing based on load factor thresholds
 * - Zero-copy semantics (raw pointer storage)
 *
 * ARCHITECTURE
 * ============
 * The engine maintains an array of hash_bucket structures, where each bucket
 * can be in one of three states:
 * 1. EMPTY: Never occupied (key == NULL, key_len == 0)
 * 2. OCCUPIED: Contains a valid key-value pair (key != NULL)
 * 3. TOMBSTONE: Previously occupied, now deleted (key == NULL, key_len != 0)
 *
 * COLLISION RESOLUTION
 * ====================
 * Linear Probing:
 * - On hash collision, the engine probes subsequent buckets sequentially
 * - Probe sequence: (hash + 0) % N, (hash + 1) % N, (hash + 2) % N, ...
 * - Stops when finding an empty slot or matching key
 * - Maximum probe length is bounded by bucket_count
 *
 * Tombstone Handling:
 * - Deleted entries are marked as tombstones (not cleared to empty)
 * - Tombstones prevent premature termination of probe chains during lookups
 * - During insertion, tombstones can be reused (first available tombstone)
 * - Resize operations discard tombstones, rehashing only active entries
 *
 * THREAD SAFETY
 * =============
 * Concurrency Model:
 * - All public API functions acquire engine->engine_lock before operation
 * - Operations are fully serialized (coarse-grained locking)
 * - Safe for concurrent access from multiple threads
 * - No reader-writer optimization (all operations are mutually exclusive)
 *
 * Lock Ordering:
 * - hash_put/hash_delete may call hash_engine_resize after unlocking
 * - hash_engine_resize re-acquires the lock for rehashing
 * - Retry loop ensures consistency after resize
 *
 * MEMORY OWNERSHIP
 * ================
 * Zero-Copy Semantics:
 * - Keys and values are NOT copied; only pointers and lengths are stored
 * - Caller retains ownership and must ensure memory validity during:
 *   * Entry lifetime (from insert until delete/overwrite)
 *   * Engine lifetime (until hash_engine_destroy)
 * - Engine does NOT free key/value memory in hash_delete or hash_engine_destroy
 *
 * Memory Accounting:
 * - total_memory tracks approximate payload size (sum of key_len + value_len)
 * - Does NOT include bucket array overhead or tombstone metadata
 * - Counter may be approximate due to concurrent operations and tombstones
 *
 * RESIZING STRATEGY
 * =================
 * Load Factor Thresholds:
 * - MAX_LOAD_FACTOR (0.75): grow when (item_count + 1) / bucket_count > 0.75
 * - MIN_LOAD_FACTOR (0.2): shrink when item_count / bucket_count < 0.2
 * - MIN_BUCKET_COUNT: minimum table size (prevents excessive shrinking)
 * - MAX_BUCKET_COUNT: maximum table size (prevents unbounded growth)
 *
 * Growth Strategy:
 * - Triggered proactively before hash_put insertion
 * - Doubles bucket_count (capped at MAX_BUCKET_COUNT)
 * - If rehash fails due to pathological collisions, grows again (2x retry)
 *
 * Shrink Strategy:
 * - Triggered after hash_delete if load factor drops below threshold
 * - Halves bucket_count (floored at MIN_BUCKET_COUNT)
 * - Best-effort (failure is tolerated, old table remains valid)
 *
 * Rehashing Process:
 * 1. Allocate new bucket array (target size)
 * 2. Lock engine and capture old table
 * 3. Recompute hash for each active entry under new size
 * 4. Use linear probing to insert into new table
 * 5. If new table fills (pathological case), grow and retry
 * 6. Swap tables and unlock
 * 7. Free old table
 *
 * HASH FUNCTION
 * =============
 * SipHash-2-4:
 * - Cryptographic-quality pseudorandom function
 * - Keyed with per-process random keys (hash_key_0, hash_key_1)
 * - Keys initialized once in hash_engine_init via siphash_init_random_key
 * - Falls back to weak (zero) keys if randomization fails
 * - Defends against hash-flooding DoS attacks
 *
 * Bucket Index Calculation:
 * - hash_value = siphash24(key, key_len, hash_key_0, hash_key_1)
 * - bucket_index = hash_value % bucket_count
 * - Modulo bias negligible for large bucket counts
 *
 * PERFORMANCE CHARACTERISTICS
 * ===========================
 * Time Complexity (average case):
 * - hash_put: O(1) amortized (includes occasional O(N) resize)
 * - hash_get: O(1) average, O(N) worst-case (long probe chain)
 * - hash_delete: O(1) average, O(N) worst-case + occasional resize
 *
 * Space Complexity:
 * - O(N) bucket array (N = bucket_count)
 * - Overhead: ~24-32 bytes per bucket (depends on pointer size)
 * - Tombstone overhead increases with delete-heavy workloads
 * - Resizing reclaims tombstone memory
 *
 * Load Factor Impact:
 * - Higher load → more collisions → longer probe chains
 * - Lower load → less memory efficient
 * - 0.75 threshold balances performance and memory
 *
 * USAGE EXAMPLE
 * =============
 * ```c
 * struct hash_engine engine;
 * hash_engine_init(&engine, 16);
 *
 * const char *key = "mykey";
 * const char *value = "myvalue";
 * hash_put(&engine, key, strlen(key), value, strlen(value));
 *
 * const void *retrieved_value;
 * size_t retrieved_len;
 * if (hash_get(&engine, key, strlen(key), &retrieved_value, &retrieved_len) ==
 * 0) { printf("Found: %.*s\n", (int)retrieved_len, (const
 * char*)retrieved_value);
 * }
 *
 * hash_delete(&engine, key, strlen(key));
 * hash_engine_destroy(&engine);
 * ```
 *
 * ERROR CODES
 * ===========
 * - 0: Success
 * - -EINVAL: Invalid parameter (e.g., bucket_count <= 0)
 * - -ENOMEM: Memory allocation failure
 * - -ENOSPC: Table cannot grow any further
 * - -ENOENT: Key not found (hash_get, hash_delete)
 *
 * LIMITATIONS
 * ===========
 * - No iterator support (cannot enumerate all entries)
 * - No batch operations (each operation acquires lock)
 * - Coarse-grained locking (no concurrent readers)
 * - No support for variable-size keys/values after insertion
 * - Tombstone accumulation degrades performance until resize
 * - Maximum capacity bounded by MAX_BUCKET_COUNT
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

#include "hash_engine.h"
#include "bucket.h"
#include "siphash.h"
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static uint64_t hash_key_0 = 0;
static uint64_t hash_key_1 = 0;

/**
 * Compute the target bucket count for growing the hash table.
 *
 * Parameters:
 * - current_count: Current number of buckets in the hash table.
 *
 * Returns:
 * - Target bucket count after growth (doubled, capped at MAX_BUCKET_COUNT).
 * - Returns current_count unchanged if already at maximum capacity.
 *
 * Notes:
 * - Growth strategy: 2x multiplication for exponential scaling.
 * - Capped at MAX_BUCKET_COUNT to prevent unbounded memory allocation.
 * - Uses long intermediate to safely detect overflow before capping.
 */
static inline int
hash_engine_grow_target(int current_count)
{
	long next;

	if (current_count >= MAX_BUCKET_COUNT)
		return current_count;

	next = (long)current_count * 2;
	if (next > MAX_BUCKET_COUNT)
		next = MAX_BUCKET_COUNT;
	return (int)next;
}

/**
 * Compute the target bucket count for shrinking the hash table.
 *
 * Parameters:
 * - current_count: Current number of buckets in the hash table.
 *
 * Returns:
 * - Target bucket count after shrinkage (halved, floored at MIN_BUCKET_COUNT).
 * - Returns current_count unchanged if already at minimum capacity.
 *
 * Notes:
 * - Shrink strategy: 0.5x division for symmetric exponential scaling.
 * - Floored at MIN_BUCKET_COUNT to maintain minimum table size.
 * - Prevents excessive memory deallocation for small tables.
 */
static inline int
hash_engine_shrink_target(int current_count)
{
	int next;

	if (current_count <= MIN_BUCKET_COUNT)
		return current_count;

	next = current_count / 2;
	if (next < MIN_BUCKET_COUNT)
		next = MIN_BUCKET_COUNT;
	return next;
}

/**
 * Initialize a hash_engine with the specified number of buckets.
 *
 * Parameters:
 * - engine: Pointer to an allocated struct hash_engine to initialize.
 * - bucket_count: Number of buckets to allocate (array of struct hash_bucket).
 *
 * Returns:
 * - 0 on success;
 * - -EINVAL if bucket_count is invalid;
 * - -ENOMEM if allocation fails;
 * - -EAGAIN/-EBUSY (mirrors pthread error codes) if mutex init fails.
 *
 * Notes:
 * - Allocates bucket storage and initializes the engine mutex and counters.
 * - Initializes SipHash keys used for hashing; may fall back to weak keys.
 * - Must be called before any other API and not concurrently with them.
 */
int
hash_engine_init(struct hash_engine *engine, int bucket_count)
{
	int success;
	int rc;

	/* Ensure a known initial state before first resize (mutex not yet
	 * init). */
	engine->hash_buckets = NULL;
	engine->bucket_count = 0;
	engine->item_count = 0;
	engine->total_memory = 0;

	success = hash_engine_resize(engine, bucket_count);
	if (success != 0) {
		return success;
	}
	// Initialize random keys for this engine instance
	if (siphash_init_random_key(&hash_key_0, &hash_key_1) != 0) {
		printf("Warning: Using weak random keys\n");
	}

	rc = pthread_mutex_init(&engine->engine_lock, NULL);
	if (rc != 0) {
		free(engine->hash_buckets);
		engine->hash_buckets = NULL;
		engine->bucket_count = 0;
		return -rc;
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
 * - 0 on success;
 * - -ENOSPC if capacity limit prevents growth;
 * - -ENOMEM if a resize allocation fails.
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
 * - 0 on success;
 * - -ENOENT if the key does not exist.
 *
 * Thread-safety:
 * - Internally synchronized; safe to call concurrently.
 *
 * Notes:
 * - Uses linear probing with first-tombstone reuse. If the key already
 *   exists in the probe chain, the entry is overwritten in place.
 * - May grow the table based on projected load; if insertion exhausts the
 *   probe sequence, the table is grown and the operation retried.
 * - Zero-copy: pointers are stored directly (no deep copy). Caller must keep
 *   key/value memory valid until deletion or overwrite.
 */
int
hash_put(struct hash_engine *engine, const void *key, size_t key_len,
	 const void *value, size_t value_len)
{
	int grow_target;
	int idx;
	int first_tombstone;
	int steps;
	int resize_target;
	int result;

retry:
	pthread_mutex_lock(&engine->engine_lock);

	grow_target = 0;
	if (engine->bucket_count < MAX_BUCKET_COUNT) {
		float projected_load = (float)(engine->item_count + 1)
				       / (float)engine->bucket_count;
		if (projected_load > MAX_LOAD_FACTOR)
			grow_target
			    = hash_engine_grow_target(engine->bucket_count);
	}

	if (grow_target > 0 && grow_target != engine->bucket_count) {
		pthread_mutex_unlock(&engine->engine_lock);
		result = hash_engine_resize(engine, grow_target);
		if (result != 0)
			return result;
		goto retry;
	}

	idx = hash_engine_hash(engine, key, key_len);
	first_tombstone = -1;
	steps = 0;

	while (steps < engine->bucket_count) {
		struct hash_bucket *b = &engine->hash_buckets[idx];

		if (bucket_is_empty(b)) {
			int insert_idx
			    = (first_tombstone != -1) ? first_tombstone : idx;
			struct hash_bucket *dst
			    = &engine->hash_buckets[insert_idx];

			// New insert (reuse tombstone if available)
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

	resize_target = hash_engine_grow_target(engine->bucket_count);
	pthread_mutex_unlock(&engine->engine_lock);
	if (resize_target == engine->bucket_count)
		return -ENOSPC;
	result = hash_engine_resize(engine, resize_target);
	if (result != 0)
		return result;
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
 * - 0 on success;
 * - -ENOENT if the key is not found.
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
	int idx;
	int steps;

	pthread_mutex_lock(&engine->engine_lock);

	idx = hash_engine_hash(engine, key, key_len);
	steps = 0;

	while (steps < engine->bucket_count) {
		struct hash_bucket *b = &engine->hash_buckets[idx];

		if (bucket_is_empty(b)) {
			// Hard miss (no further cluster)
			pthread_mutex_unlock(&engine->engine_lock);
			return -ENOENT;
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
	return -ENOENT;
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
 * - Uses linear probing to locate and delete the matching key; converts the
 *   bucket to a tombstone to preserve probe chain correctness.
 * - May shrink the table if the post-delete load factor is below the
 *   threshold.
 * - Does not free caller-owned key/value memory.
 */
int
hash_delete(struct hash_engine *engine, const void *key, size_t key_len)
{
	int shrink_target = 0;
	int current_count = 0;
	int idx;
	int steps;

	pthread_mutex_lock(&engine->engine_lock);

	idx = hash_engine_hash(engine, key, key_len);
	steps = 0;

	while (steps < engine->bucket_count) {
		struct hash_bucket *b = &engine->hash_buckets[idx];

		if (bucket_is_empty(b)) {
			pthread_mutex_unlock(&engine->engine_lock);
			return -ENOENT;
		}
		if (!bucket_is_tombstone(b) && b->key_len == key_len
		    && memcmp(b->key, key, key_len) == 0) {
			// Adjust counters before tombstone
			engine->item_count--;
			engine->total_memory
			    -= (int)(b->key_len + b->value_len);
			bucket_make_tombstone(b);

			current_count = engine->bucket_count;
			if (current_count > MIN_BUCKET_COUNT) {
				float load_factor = (float)engine->item_count
						    / (float)current_count;
				if (load_factor < MIN_LOAD_FACTOR)
					shrink_target
					    = hash_engine_shrink_target(
						current_count);
			}

			pthread_mutex_unlock(&engine->engine_lock);
			if (shrink_target > 0 && shrink_target != current_count)
				(void)hash_engine_resize(engine, shrink_target);
			return 0;
		}

		idx = (idx + 1) % engine->bucket_count;
		steps++;
	}

	pthread_mutex_unlock(&engine->engine_lock);
	return -ENOENT;
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
	uint64_t hash = siphash(key, key_len, hash_key_0, hash_key_1);
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
 * - 0 on success;
 * - -EINVAL if new_bucket_count is invalid;
 * - -ENOMEM if allocation fails during resize.
 *
 * Thread-safety:
 * - Rehash path is serialized with engine_lock.
 * - Supports an early initialization path before engine_lock is initialized.
 *
 * Notes:
 * - Recomputes each active entry's bucket under the new size and reinserts
 *   using linear probing; tombstones are discarded.
 * - If rehash cannot place an entry (pathological clustering), grows the
 *   target table and retries until successful.
 * - Does not allocate/copy key/value payloads; ownership remains with the
 *   caller.
 * - Counters (item_count, total_memory) are preserved.
 * - Frees the old bucket array after swapping in the new one.
 */
int
hash_engine_resize(struct hash_engine *engine, int new_bucket_count)
{
	struct hash_bucket *new_buckets = NULL, *old_buckets;
	int old_count, i;
	int target_count;

	if (new_bucket_count <= 0)
		return -EINVAL;

	new_buckets = calloc(new_bucket_count, sizeof(struct hash_bucket));
	if (!new_buckets)
		return -ENOMEM;

	/* First allocation path: init may call resize before mutex is init. */
	if (!engine->hash_buckets || engine->bucket_count == 0) {
		engine->hash_buckets = new_buckets;
		engine->bucket_count = new_bucket_count;
		return 0;
	}

	pthread_mutex_lock(&engine->engine_lock);

	old_buckets = engine->hash_buckets;
	old_count = engine->bucket_count;
	target_count = new_bucket_count;

	/* Attempt to rehash into new_buckets; grow and retry if it fills up. */
	for (;;) {
		// Reinsert with linear probing
		int failed = 0;

		// Ensure new_buckets matches target_count and is zeroed
		// (first iteration already allocated; later iterations re-alloc
		// below)
		for (i = 0; i < old_count; i++) {
			struct hash_bucket *ob;
			uint64_t h;
			int idx;
			int steps;

			ob = &old_buckets[i];
			if (!ob->key || bucket_is_tombstone(ob))
				continue;

			h = siphash(ob->key, ob->key_len, hash_key_0,
				    hash_key_1);
			idx = (int)(h % (uint64_t)target_count);
			steps = 0;

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
			return -ENOMEM;
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
 * - engine: Target engine to evaluate.
 *
 * Returns:
 * - 1 if resize is needed (either grow or shrink).
 * - 0 if current size is appropriate for the load factor.
 *
 * Thread-safety:
 * - Not thread-safe; caller must hold engine_lock or ensure exclusive access.
 *
 * Notes:
 * - Growth condition: load_factor > MAX_LOAD_FACTOR (0.75) and not at maximum
 *   capacity.
 * - Shrink condition: load_factor < MIN_LOAD_FACTOR (0.2) and not at minimum
 *   capacity.
 * - Load factor computed as item_count / bucket_count.
 * - Does not trigger resize; only reports whether one is recommended.
 */
int
needs_resize(struct hash_engine *engine)
{
	float load_factor
	    = (float)engine->item_count / (float)engine->bucket_count;

	if (load_factor > MAX_LOAD_FACTOR
	    && engine->bucket_count < MAX_BUCKET_COUNT)
		return 1;

	if (load_factor < MIN_LOAD_FACTOR
	    && engine->bucket_count > MIN_BUCKET_COUNT)
		return 1;

	return 0;
}
