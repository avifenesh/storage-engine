#ifndef SIPHASH_H
#define SIPHASH_H

/**
 * @file siphash.h
 * @brief Public API for SipHash-2-4 keyed hashing (cryptographic PRF).
 *
 * Overview:
 * - Implements SipHash-2-4 (Fast, secure, and SIMDable pseudorandom function)
 *   for hashing variable-length inputs with a 128-bit secret key.
 * - Provides core hashing functions, convenient key management, and global-key
 *   helpers.
 * - Suitable for hash table key hashing with defense against hash-flooding
 *   attacks.
 *
 * Core Functions:
 * - siphash(): Core SipHash-2-4 using two 64-bit key words (k0, k1).
 * - siphash_key(): Convenience wrapper accepting a contiguous 16-byte key.
 * - siphash_with_global_key(): Process-global key (after set-once
 *   initialization).
 *
 * Key Management:
 * - siphash_init_random_key(): Generate random key from /dev/urandom (or
 *   fallback).
 * - siphash_set_key(): Pin a process-global key (typically called once at
 *   startup).
 *
 * Security Notes:
 * - The 128-bit key must be kept secret; if disclosed, the hash becomes
 *   predictable.
 * - Suitable for general-purpose hash tables and DoS mitigation.
 *
 * Thread-safety:
 * - Pure functions (siphash, siphash_key) are thread-safe.
 * - Global-key helpers are safe for repeated use after initialization;
 *   call siphash_set_key() once at startup to ensure thread-safety.
 * - The first call to siphash_with_global_key() may race with concurrent
 *   first calls (auto-initialization).
 *
 * Performance:
 * - ~1.3 cycles per byte on modern CPUs (e.g., x86-64, ARM64).
 * - No platform-specific optimizations in this userspace stub; NEON variants
 *   planned for Sprint 4.
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