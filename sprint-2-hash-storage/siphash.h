#ifndef SIPHASH_H
#define SIPHASH_H

/**
 * @file siphash.h
 * @brief Public API for SipHash-2-4: a fast, keyed 64-bit PRF for short inputs.
 *
 * Overview:
 * - Provides SipHash-2-4 core functions and simple key management helpers.
 * - Suitable for hash table key hashing with a secret key to mitigate
 * hash-flooding.
 *
 * Keying:
 * - The 128-bit secret key is provided as two 64-bit words (k0, k1) or a
 * 16-byte array.
 * - Callers must keep keys secret; disclosure weakens collision resistance.
 *
 * Thread-safety:
 * - Pure functions (siphash, siphash_key) are thread-safe.
 * - Global-key helpers are safe for repeated use after initialization; the very
 * first auto-initialization in siphash_with_global_key is not guaranteed to be
 * race-free.
 */

#include <stddef.h>
#include <stdint.h>

// Core SipHash-2-4 API
/**
 * @brief Compute SipHash-2-4 of the input using a 128-bit key split into two
 * words.
 *
 * @param data Pointer to input bytes (may be NULL if len == 0).
 * @param len  Number of input bytes.
 * @param k0   Low 64 bits of the secret key.
 * @param k1   High 64 bits of the secret key.
 * @return 64-bit SipHash-2-4 value.
 */
uint64_t siphash(const void *data, size_t len, uint64_t k0, uint64_t k1);

/**
 * @brief Compute SipHash-2-4 using a contiguous 16-byte key.
 *
 * Key layout:
 * - key[0..7] as little-endian k0, key[8..15] as little-endian k1.
 *
 * @param data Pointer to input bytes (may be NULL if len == 0).
 * @param len  Number of input bytes.
 * @param key  Pointer to 16 secret key bytes.
 * @return 64-bit SipHash-2-4 value.
 */
uint64_t siphash_key(const void *data, size_t len, const uint8_t key[16]);

// Key management helpers
/**
 * @brief Initialize a random 128-bit SipHash key into k0/k1.
 *
 * Attempts to read from a secure OS RNG (e.g., /dev/urandom). Falls back to
 * weaker randomness if unavailable.
 *
 * @param k0 Out: low 64 bits of key.
 * @param k1 Out: high 64 bits of key.
 * @return 0 on strong randomness; -1 if a weaker fallback or error occurred.
 */
int siphash_init_random_key(uint64_t *k0, uint64_t *k1);

/**
 * @brief Set process-global SipHash key for siphash_with_global_key().
 *
 * Call once at startup to pin the global key for this process.
 *
 * @param k0 Low 64 bits of key.
 * @param k1 High 64 bits of key.
 */
void siphash_set_key(uint64_t k0, uint64_t k1);

/**
 * @brief Compute SipHash-2-4 using a process-global key.
 *
 * If the global key is not set, attempts to auto-initialize it with randomness.
 * Not guaranteed race-free on first use in highly concurrent contexts.
 *
 * @param data Pointer to input bytes (may be NULL if len == 0).
 * @param len  Number of input bytes.
 * @return 64-bit SipHash-2-4 value.
 */
uint64_t siphash_with_global_key(const void *data, size_t len);

#endif