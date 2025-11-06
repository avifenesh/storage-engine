/**
 * @file bucket.h
 * @brief Public API for bucket state management in open addressing.
 *
 * Overview:
 * - Provides predicates and state-modification functions for the three bucket
 *   states used in the hash engine's linear probing scheme.
 *
 * Bucket States (Encoding):
 * - Empty (never written): key == NULL && key_len == 0
 * - Tombstone (deleted): key == NULL && key_len != 0
 * - Active (valid entry): key != NULL
 *
 * Usage in Hash Engine:
 * - hash_put(): Uses bucket_is_empty() to locate insertion point; tracks
 *   first bucket_is_tombstone() for potential reuse.
 * - hash_get(): Probes past tombstones (bucket_is_tombstone()), stops at
 *   bucket_is_empty().
 * - hash_delete(): Converts active bucket to tombstone via
 *   bucket_make_tombstone().
 * - hash_engine_resize(): Skips tombstones during rehash; only copies active
 *   entries to new table.
 *
 * Thread-safety:
 * - These helpers do not perform synchronization. Callers must ensure
 *   exclusive access to the bucket (e.g., hold the engine lock) when modifying
 *   or inspecting shared state.
 *
 * Design Rationale:
 * - State is encoded in existing fields (key pointer and key_len) to avoid
 *   adding a separate flag byte.
 * - Functions are simple and suitable for inlining or integration into caller
 *   loops.
 */

#ifndef BUCKET_H
#define BUCKET_H

#include "hash_engine.h"
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
int bucket_is_empty(const struct hash_bucket *bucket);

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
 * - Callers should hold appropriate synchronization when reading/modifying
 *   buckets.
 */
int bucket_is_tombstone(const struct hash_bucket *bucket);

/**
 * Mark a bucket as a tombstone (deleted).
 *
 * Parameters:
 * - bucket: Pointer to bucket to mark.
 *
 * Notes:
 * - Sets key to NULL but preserves key_len as deletion marker.
 * - Allows linear probing to continue past this slot during search.
 * - Leaves value/value_len untouched; they are ignored once a tombstone.
 * - Does not free any caller-owned memory.
 */
void bucket_make_tombstone(struct hash_bucket *bucket);

#endif /* BUCKET_H */
