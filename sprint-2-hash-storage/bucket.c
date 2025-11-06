/**
 * @file bucket.c
 * @brief Bucket helper implementations for state management in open addressing.
 *
 * Overview:
 * - Provides predicates and state-modification functions for managing the
 *   three bucket states in the hash engine's linear probing scheme.
 *
 * Bucket States (Encoding):
 * - Empty (never written): key == NULL && key_len == 0
 *   - Signals end of probe chain; search stops here.
 * - Tombstone (deleted): key == NULL && key_len != 0
 *   - Allows probe to continue; reusable for new insertions (optimization).
 * - Active (valid): key != NULL
 *   - Contains a stored entry; may be a match or start of a collision chain.
 *
 * Correctness Notes:
 * - Tombstones are essential for linear probing: without them, deletion creates
 *   "holes" that prematurely terminate subsequent searches on the same chain.
 * - During resize/rehash, only active entries are copied; tombstones are
 *   discarded, reclaiming wasted space.
 *
 * Thread-safety:
 * - No internal synchronization; callers must ensure exclusive access (e.g.,
 *   hold the engine's mutex) before modifying or inspecting buckets.
 *
 * Design:
 * - State encoding uses existing hash_bucket fields (key and key_len) to avoid
 *   additional overhead (no separate "flags" field).
 * - Functions are simple predicates and mutators, suitable for inlining.
 */

#include "bucket.h"
#include <stddef.h>

/**
 * Check if a bucket is empty (never written).
 *
 * Parameters:
 * - bucket: Pointer to bucket to check.
 *
 * Returns:
 * - Non-zero if bucket contains no entry; zero otherwise.
 *
 * Notes:
 * - Empty buckets terminate linear probe chains.
 */
int
bucket_is_empty(const struct hash_bucket *bucket)
{
	return bucket->key == NULL && bucket->key_len == 0;
}

/**
 * Check if a bucket is a tombstone (deleted entry).
 *
 * Parameters:
 * - bucket: Pointer to bucket to check.
 *
 * Returns:
 * - Non-zero if bucket is marked deleted; zero otherwise.
 *
 * Notes:
 * - Tombstones allow linear probing to continue past deleted slots.
 * - Encoded as key == NULL with non-zero key_len.
 */
int
bucket_is_tombstone(const struct hash_bucket *bucket)
{
	return bucket->key == NULL && bucket->key_len != 0;
}

/**
 * Mark a bucket as a tombstone (deleted).
 *
 * Parameters:
 * - bucket: Pointer to bucket to mark.
 *
 * Notes:
 * - Sets key to NULL but preserves key_len as deletion marker.
 * - Allows linear probing to continue past this slot during search.
 * - Leaves value/value_len untouched; they are ignored after marking.
 * - Does not free any caller-owned memory.
 */
void
bucket_make_tombstone(struct hash_bucket *bucket)
{
	bucket->key = NULL;
	/* key_len remains non-zero to mark tombstone */
}
