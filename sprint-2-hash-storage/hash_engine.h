/**
 * @file hash_engine.h
 * @brief Public API for a SipHash-based hash table with linear probing.
 *
 * Overview:
 * - Provides dynamic key/value storage using a bucket array with linear probing
 *   for collision resolution and tombstones for correct deletion semantics.
 * - Bucket array grows/shrinks based on load factor thresholds to maintain
 *   O(1) average-case operations.
 * - Uses SipHash-2-4 with per-process random keys to hash keys to bucket
 *   indices.
 * - Stores raw key/value pointers (caller-owned); no deep copying.
 *
 * Collision Resolution:
 * - Linear probing: on collision, probe the next bucket (wrapping at array
 *   end).
 * - Tombstones: deleted entries are marked (key == NULL, key_len != 0) to
 *   allow probe chains to continue past deleted slots.
 * - Resize/rehash: moves all active entries to a new array; tombstones are
 *   discarded.
 *
 * Thread-safety:
 * - All public API functions are synchronized with an internal mutex.
 * - Concurrent calls to the same engine are safe; operations are serialized.
 *
 * Ownership and lifetime:
 * - Keys and values are not copied; only pointers and lengths are stored.
 *   Callers must ensure memory remains valid until deletion or engine
 *   destruction.
 *
 * Error handling:
 * - Functions return 0 on success; negative errno-like codes on failure
 *   (e.g., -EINVAL for bad args, -ENOMEM for allocation failure,
 *   -ENOENT for not found).
 *
 * Load Factor Tuning:
 * - Resize occurs when load factor > MAX_LOAD_FACTOR (0.75) or
 *   < MIN_LOAD_FACTOR (0.2).
 * - Bucket count bounds: MIN_BUCKET_COUNT (16) to MAX_BUCKET_COUNT
 *   (1048576).
 */

#ifndef HASH_ENGINE_H
#define HASH_ENGINE_H

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Load factor thresholds used by resize policies.
 *
 * @def MAX_LOAD_FACTOR
 *      Upper threshold that may trigger an expansion when item density is high.
 * @def MIN_LOAD_FACTOR
 *      Lower threshold that may trigger a contraction when sparsity is high.
 */
#define MAX_LOAD_FACTOR 0.75
#define MIN_LOAD_FACTOR 0.2

/**
 * Default and bounds for bucket array sizing.
 *
 * @def DEFAULT_BUCKET_COUNT
 *      Recommended default number of buckets when omitted by callers.
 * @def INITIAL_BUCKET_COUNT
 *      Minimal viable starting size used by some constructors.
 * @def MAX_BUCKET_COUNT
 *      Hard upper bound to prevent excessive allocation.
 * @def MIN_BUCKET_COUNT
 *      Hard lower bound to maintain basic functionality.
 */
#define DEFAULT_BUCKET_COUNT 1024
#define INITIAL_BUCKET_COUNT 16
#define MAX_BUCKET_COUNT 1048576
#define MIN_BUCKET_COUNT 16

/**
 * @struct hash_bucket
 * @brief Node representing a single key/value entry in an open-addressed table.
 *
 * @var hash_bucket::key
 *      Pointer to caller-owned key bytes (not copied).
 * @var hash_bucket::key_len
 *      Length of the key in bytes.
 * @var hash_bucket::value
 *      Pointer to caller-owned value bytes (not copied).
 * @var hash_bucket::value_len
 *      Length of the value in bytes.
 * @var hash_bucket::next
 *      Optional link for chaining-based strategies; unused by the current
 *      linear probing implementation.
 */
struct hash_bucket {
	const void *key;
	size_t key_len;
	const void *value;
	size_t value_len;
	struct hash_bucket *next;
};

/**
 * @struct hash_engine
 * @brief Hash engine instance and its shared state.
 *
 * @var hash_engine::hash_buckets
 *      Pointer to the bucket array storage.
 * @var hash_engine::bucket_count
 *      Current number of buckets.
 * @var hash_engine::engine_lock
 *      Mutex guarding shared state for thread-safety.
 * @var hash_engine::item_count
 *      Approximate number of stored items.
 * @var hash_engine::total_memory
 *      Approximate sum of key and value lengths stored.
 */
struct hash_engine {
	struct hash_bucket *hash_buckets;
	int bucket_count;
	pthread_mutex_t engine_lock;
	int item_count;
	int total_memory;
};

/**
 * @brief Initialize a hash_engine with the specified bucket count.
 *
 * @param engine        Pointer to an allocated engine struct to initialize.
 * @param bucket_count  Number of buckets to allocate.
 * @return 0 on success; negative errno on failure (e.g., -EINVAL, -ENOMEM,
 *         -EAGAIN).
 *
 * @thread_safety Not thread-safe with concurrent use of the same engine
 *                before initialization completes.
 */
int hash_engine_init(struct hash_engine *engine, int bucket_count);

/**
 * @brief Compute a bucket index for a given key.
 *
 * @param engine   Target engine (may provide hashing seed/context).
 * @param key      Pointer to key bytes.
 * @param key_len  Length of key in bytes.
 * @return Non-negative bucket index in the range [0, engine->bucket_count).
 *
 * @note Does not validate input pointers. Safe to call concurrently.
 */
int hash_engine_hash(struct hash_engine *engine, const void *key,
		     size_t key_len);

/**
 * @brief Resize the engine to a new bucket count, rehashing all entries.
 *
 * @param engine           Target engine.
 * @param new_bucket_count Desired new number of buckets.
 * @return 0 on success; -EINVAL for invalid counts; -ENOMEM on allocation
 *         failure (no change to the table on failure).
 *
 * @thread_safety Uses internal locking; not re-entrant on the same engine.
 * @details Rehash reinserts active entries using linear probing and discards
 *          tombstones. On pathological clustering, grows the target size and
 *          retries until successful. May be O(n).
 */
int hash_engine_resize(struct hash_engine *engine, int new_bucket_count);

/**
 * @brief Insert or overwrite a key/value pair.
 *
 * @param engine     Target engine.
 * @param key        Pointer to key bytes (caller-owned).
 * @param key_len    Length of key in bytes.
 * @param value      Pointer to value bytes (caller-owned).
 * @param value_len  Length of value in bytes.
 * @return 0 on success; -ENOSPC if the table is at capacity; -ENOMEM if a
 *         resize allocation fails.
 *
 * @note No deep copies are performed; caller must manage memory lifetimes.
 * @details Linear probing with tombstone reuse; overwrites value if the key
 *          already exists in the probe chain. May proactively grow and retry
 *          when projected load exceeds thresholds.
 */
int hash_put(struct hash_engine *engine, const void *key, size_t key_len,
	     const void *value, size_t value_len);

/**
 * @brief Retrieve a stored value for a key.
 *
 * @param engine     Target engine.
 * @param key        Pointer to key bytes.
 * @param key_len    Length of key in bytes.
 * @param value      Out pointer receiving stored value pointer (not copied).
 * @param value_len  Out pointer receiving stored value length.
 * @return 0 on success; -ENOENT if the key is not present.
 *
 * @note Returned pointers reference caller-owned memory.
 */
int hash_get(struct hash_engine *engine, const void *key, size_t key_len,
	     const void **value, size_t *value_len);

/**
 * @brief Delete the entry for a given key.
 *
 * @param engine   Target engine.
 * @param key      Pointer to key bytes.
 * @param key_len  Length of key in bytes.
 * @return 0 on success; -ENOENT if the key is not present.
 *
 * @note Does not free key/value memory; caller remains responsible.
 */
int hash_delete(struct hash_engine *engine, const void *key, size_t key_len);

/**
 * @brief Destroy a previously initialized hash_engine and release resources.
 *
 * @param engine Target engine.
 * @return 0 on success.
 *
 * @thread_safety Not safe with concurrent operations on the same engine.
 * @note Does not free any caller-owned key/value memory.
 */
int hash_engine_destroy(struct hash_engine *engine);

/**
 * @brief Retrieve engine statistics.
 *
 * @param engine        Target engine.
 * @param item_count    Out pointer for number of stored items (nullable).
 * @param bucket_count  Out pointer for number of buckets (nullable).
 * @param memory_usage  Out pointer for approximate total bytes (nullable).
 * @return 0 on success.
 *
 * @thread_safety Safe to call concurrently; values reflect a point-in-time
 * snapshot.
 */
int hash_engine_get_stats(struct hash_engine *engine, uint32_t *item_count,
			  uint32_t *bucket_count, uint32_t *memory_usage);
/**
 * @brief Determine if the engine needs resizing based on load factors.
 *
 * @param engine Target engine.
 * @return Non-zero if resizing is recommended; zero otherwise.
 *
 * @thread_safety Not thread-safe; caller must hold engine_lock.
 */
int needs_resize(struct hash_engine *engine);
#endif
