#include "siphash.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stddef.h>

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
#define SIPROUND \
    do { \
        v0 += v1; v1 = ROTL(v1, 13); v1 ^= v0; v0 = ROTL(v0, 32); \
        v2 += v3; v3 = ROTL(v3, 16); v3 ^= v2; \
        v0 += v3; v3 = ROTL(v3, 21); v3 ^= v0; \
        v2 += v1; v1 = ROTL(v1, 17); v1 ^= v2; v2 = ROTL(v2, 32); \
    } while(0)

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
static inline uint64_t read64le(const uint8_t *p) {
    return (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) |
           ((uint64_t)p[3] << 24) | ((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40) |
           ((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56);
}

// Global keys (initialized once at startup)
static uint64_t global_k0 = 0;
static uint64_t global_k1 = 0;
static int keys_initialized = 0;

// Generate cryptographically secure random keys
int siphash_init_random_key(uint64_t *k0, uint64_t *k1) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        // Fallback to weaker randomness
        srand((unsigned int)time(NULL) ^ getpid());
        *k0 = ((uint64_t)rand() << 32) | rand();
        *k1 = ((uint64_t)rand() << 32) | rand();
        return -1; // Indicate weak randomness
    }
    
    uint8_t key_bytes[16];
    ssize_t bytes_read = read(fd, key_bytes, 16);
    close(fd);
    
    if (bytes_read != 16) {
        return -1;
    }
    
    *k0 = read64le(key_bytes);
    *k1 = read64le(key_bytes + 8);
    return 0;
}

// Set global keys (call once at startup)
void siphash_set_key(uint64_t k0, uint64_t k1) {
    global_k0 = k0;
    global_k1 = k1;
    keys_initialized = 1;
}

// Convenience function using global keys
uint64_t siphash_with_global_key(const void *data, size_t len) {
    if (!keys_initialized) {
        // Auto-initialize with random keys if not set
        uint64_t k0, k1;
        siphash_init_random_key(&k0, &k1);
        siphash_set_key(k0, k1);
    }
    return siphash(data, len, global_k0, global_k1);
}

uint64_t siphash(const void *data, size_t len, uint64_t k0, uint64_t k1)
{
    const uint8_t *in = (const uint8_t *)data;

    // Initialize state with magic constants XORed with key
    uint64_t v0 = 0x736f6d6570736575ULL ^ k0;
    uint64_t v1 = 0x646f72616e646f6dULL ^ k1;
    uint64_t v2 = 0x6c7967656e657261ULL ^ k0;
    uint64_t v3 = 0x7465646279746573ULL ^ k1;

    // Process full 8-byte blocks
    const uint8_t *end = in + len - (len % 8);
    for (; in != end; in += 8) {
        uint64_t m = read64le(in);
        v3 ^= m;
        SIPROUND; SIPROUND;  // 2 compression rounds
        v0 ^= m;
    }

    // Handle remaining bytes + length
    uint64_t b = ((uint64_t)len) << 56;
    switch (len & 7) {
        case 7: b |= ((uint64_t)in[6]) << 48;
        case 6: b |= ((uint64_t)in[5]) << 40;
        case 5: b |= ((uint64_t)in[4]) << 32;
        case 4: b |= ((uint64_t)in[3]) << 24;
        case 3: b |= ((uint64_t)in[2]) << 16;
        case 2: b |= ((uint64_t)in[1]) << 8;
        case 1: b |= ((uint64_t)in[0]);
        case 0: break;
    }

    v3 ^= b;
    SIPROUND; SIPROUND;  // 2 more compression rounds
    v0 ^= b;

    // Finalization
    v2 ^= 0xff;
    SIPROUND; SIPROUND; SIPROUND; SIPROUND;  // 4 finalization rounds

    return v0 ^ v1 ^ v2 ^ v3;
}

uint64_t siphash_key(const void *data, size_t len, const uint8_t key[16])
{
    uint64_t k0 = read64le(key);
    uint64_t k1 = read64le(key + 8);
    return siphash(data, len, k0, k1);
}
