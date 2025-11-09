/**
 * @file siphash.h
 * @brief SipHash-2-4 cryptographic hash function
 *
 * SipHash is a fast, cryptographically strong hash function designed for
 * hash table lookups and other applications requiring resistance to hash
 * flooding attacks.
 */

#ifndef STORAGE_HASH_SIPHASH_H
#define STORAGE_HASH_SIPHASH_H

#include <stddef.h>
#include <stdint.h>

/**
 * Initialize random key from /dev/urandom or fallback to rand()
 *
 * @param k0 Pointer to store first 64-bit key
 * @param k1 Pointer to store second 64-bit key
 * @return 0 on success (urandom), -1 on fallback to rand()
 */
int siphash_init_random_key(uint64_t *k0, uint64_t *k1);

/**
 * Set global key for siphash_with_global_key()
 *
 * @param k0 First 64-bit key
 * @param k1 Second 64-bit key
 */
void siphash_set_key(uint64_t k0, uint64_t k1);

/**
 * Compute SipHash using global key (initialized on first call)
 *
 * @param data Input data to hash
 * @param len Length of input data in bytes
 * @return 64-bit hash value
 */
uint64_t siphash_with_global_key(const void *data, size_t len);

/**
 * Compute SipHash-2-4 with explicit keys
 *
 * @param data Input data to hash
 * @param len Length of input data in bytes
 * @param k0 First 64-bit key
 * @param k1 Second 64-bit key
 * @return 64-bit hash value
 */
uint64_t siphash(const void *data, size_t len, uint64_t k0, uint64_t k1);

/**
 * Compute SipHash with 16-byte key array
 *
 * @param data Input data to hash
 * @param len Length of input data in bytes
 * @param key 16-byte key (interpreted as two 64-bit little-endian values)
 * @return 64-bit hash value
 */
uint64_t siphash_key(const void *data, size_t len, const uint8_t key[16]);

#endif /* STORAGE_HASH_SIPHASH_H */
