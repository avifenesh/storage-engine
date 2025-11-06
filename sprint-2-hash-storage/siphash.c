/**
 * @file siphash.c
 * @brief SipHash-2-4 implementation: secure keyed hashing for hash tables.
 *
 * Overview:
 * - Implements SipHash-2-4 (a fast cryptographic PRF) as specified in:
 *   https://siphash.org
 *
 * Algorithm Summary:
 * - Processes input in 8-byte (64-bit) little-endian blocks.
 * - 2 compression rounds per block + 4 finalization rounds.
 * - Resists length-extension and collision-finding attacks.
 * - Uses a 128-bit key (split into k0, k1) and four 64-bit state variables.
 *
 * Key Management:
 * - Per-process random keys: initialized via siphash_init_random_key() and
 *   pinned with siphash_set_key().
 * - Global keys are stored as static module state; thread-safe after
 *   initialization. The first lazy initialization via
 *   siphash_with_global_key() is not guaranteed race-free in highly
 *   concurrent starts.
 *
 * Platform Notes:
 * - Endianness-agnostic; read64le() explicitly converts bytes to little-endian.
 * - No alignment requirements; loads occur byte-by-byte.
 * - No platform-specific SIMD in this userspace stub; planned for Sprint 4.
 *
 * Integration with Hash Engine:
 * - Called by hash_engine_hash() to map (key, key_len) to bucket indices.
 * - Keys are initialized once in hash_engine_init().
 */

#include "siphash.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Rotate left macro
// bit view step by step:
// Example (toy 8-bit width for illustration): x = 0b10110011, b = 3
//  - left  shift: (x << 3)      = 0b10011000
//  - right shift: (x >> (8-3))  = 0b00000101
//  - wrap with OR:               0b10011000 | 0b00000101 = 0b10011101
// So ROTL(x, 3) = 0b10011101.
// Notes (64-bit):
//  - ROTL(x, 32) swaps the high/low 32-bit halves.
//  - ROTL(x, 16) rotates by two bytes.
//  - For arbitrary b: (x << b) | (x >> (64 - b)).
#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

// SipRound - core mixing operation
#define SIPROUND                                                               \
	do {                                                                   \
		v0 += v1;                                                      \
		v1 = ROTL(v1, 13);                                             \
		v1 ^= v0;                                                      \
		v0 = ROTL(v0, 32);                                             \
		v2 += v3;                                                      \
		v3 = ROTL(v3, 16);                                             \
		v3 ^= v2;                                                      \
		v0 += v3;                                                      \
		v3 = ROTL(v3, 21);                                             \
		v3 ^= v0;                                                      \
		v2 += v1;                                                      \
		v1 = ROTL(v1, 17);                                             \
		v1 ^= v2;                                                      \
		v2 = ROTL(v2, 32);                                             \
	} while (0)

// One SipRound (toy 8-bit walkthrough; real impl is 64-bit)
// Rotations reduced mod 8: 13→5, 16→0, 21→5, 17→1, 32→0
// Start:
//  v0=0x35 (00110101)  v1=0x9A (10011010)
//  v2=0x4F (01001111)  v3=0x17 (00010111)
// Steps:
//  1) v0 += v1  -> v0=0xCF (11001111)
//  2) v1 = ROTL(v1,5) -> 0x53 (01010011)
//  3) v1 ^= v0  -> 0x9C (10011100)
//  4) v0 = ROTL(v0,0) -> 0xCF (unchanged)
//  5) v2 += v3  -> 0x66 (01100110)
//  6) v3 = ROTL(v3,0) -> 0x17 (unchanged)
//  7) v3 ^= v2  -> 0x71 (01110001)
//  8) v0 += v3  -> 0x40 (wrap mod 256)
//  9) v3 = ROTL(v3,5) -> 0x2E (00101110)
// 10) v3 ^= v0  -> 0x6E (01101110)
// 11) v2 += v1  -> 0x02 (wrap mod 256)
// 12) v1 = ROTL(v1,1) -> 0x39 (00111001)
// 13) v1 ^= v2  -> 0x3B (00111011)
// 14) v2 = ROTL(v2,0) -> 0x02 (unchanged)
// Result (toy): v0=0x40, v1=0x3B, v2=0x02, v3=0x6E

// Read 64-bit little-endian

/**
 * @brief Assemble a 64-bit unsigned integer from 8 bytes in little-endian
 * order.
 *
 * Constructs a uint64_t by combining bytes p[0]..p[7] where p[0] is the least
 * significant byte and p[7] is the most significant byte.
 *
 * Preconditions:
 * - p must be non-NULL and point to at least 8 readable bytes.
 *
 * Notes:
 * - Endianness-agnostic: produces the same value on little- or big-endian
 * hosts.
 * - Alignment-safe: loads occur byte-by-byte (no strict-aliasing or alignment
 * requirements).
 * - No bounds or NULL checks are performed.
 *
 * @param p Pointer to the first of 8 bytes in little-endian order.
 * @return The 64-bit unsigned value represented by the 8 bytes.
 */
static inline uint64_t
read64le(const uint8_t *p)
{
	return (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16)
	       | ((uint64_t)p[3] << 24) | ((uint64_t)p[4] << 32)
	       | ((uint64_t)p[5] << 40) | ((uint64_t)p[6] << 48)
	       | ((uint64_t)p[7] << 56);
}

// Global keys (initialized once at startup)
static uint64_t global_k0 = 0;
static uint64_t global_k1 = 0;
static int keys_initialized = 0;

// Generate cryptographically secure random keys
int
siphash_init_random_key(uint64_t *k0, uint64_t *k1)
{
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd >= 0) {
		uint8_t key_bytes[16];
		ssize_t bytes_read = read(fd, key_bytes, 16);
		close(fd);
		if (bytes_read == 16) {
			*k0 = read64le(key_bytes);
			*k1 = read64le(key_bytes + 8);
			return 0; /* strong randomness */
		}
	}

	/* Fallback to weaker randomness on any failure path. */
	srand((unsigned int)time(NULL) ^ getpid());
	*k0 = ((uint64_t)rand() << 32) | (uint64_t)rand();
	*k1 = ((uint64_t)rand() << 32) | (uint64_t)rand();
	return -1; /* indicate weak randomness used */
}

// Set global keys (call once at startup)
void
siphash_set_key(uint64_t k0, uint64_t k1)
{
	global_k0 = k0;
	global_k1 = k1;
	keys_initialized = 1;
}

// Convenience function using global keys
uint64_t
siphash_with_global_key(const void *data, size_t len)
{
	if (!keys_initialized) {
		// Auto-initialize with random keys if not set
		uint64_t k0, k1;
		siphash_init_random_key(&k0, &k1);
		siphash_set_key(k0, k1);
	}
	return siphash(data, len, global_k0, global_k1);
}

uint64_t
siphash(const void *data, size_t len, uint64_t k0, uint64_t k1)
{
	const uint8_t *in = (const uint8_t *)data;
	const uint8_t *end;
	uint64_t v0, v1, v2, v3;
	uint64_t m;
	uint64_t b;

	// Initialize state with magic constants XORed with key
	v0 = 0x736f6d6570736575ULL ^ k0;
	v1 = 0x646f72616e646f6dULL ^ k1;
	v2 = 0x6c7967656e657261ULL ^ k0;
	v3 = 0x7465646279746573ULL ^ k1;

	// Process full 8-byte blocks
	end = in + len - (len % 8);
	for (; in != end; in += 8) {
		m = read64le(in);
		v3 ^= m;
		SIPROUND;
		SIPROUND; // 2 compression rounds
		v0 ^= m;
	}

	// Handle remaining bytes + length
	b = ((uint64_t)len) << 56;
	switch (len & 7) {
	case 7:
		b |= ((uint64_t)in[6]) << 48;
		/* fallthrough */
	case 6:
		b |= ((uint64_t)in[5]) << 40;
		/* fallthrough */
	case 5:
		b |= ((uint64_t)in[4]) << 32;
		/* fallthrough */
	case 4:
		b |= ((uint64_t)in[3]) << 24;
		/* fallthrough */
	case 3:
		b |= ((uint64_t)in[2]) << 16;
		/* fallthrough */
	case 2:
		b |= ((uint64_t)in[1]) << 8;
		/* fallthrough */
	case 1:
		b |= ((uint64_t)in[0]);
		/* fallthrough */
	case 0:
		break;
	}

	v3 ^= b;
	SIPROUND;
	SIPROUND; // 2 more compression rounds
	v0 ^= b;

	// Finalization
	v2 ^= 0xff;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	SIPROUND; // 4 finalization rounds

	return v0 ^ v1 ^ v2 ^ v3;
}

uint64_t
siphash_key(const void *data, size_t len, const uint8_t key[16])
{
	uint64_t k0 = read64le(key);
	uint64_t k1 = read64le(key + 8);
	return siphash(data, len, k0, k1);
}
