/**
 * @file siphash.c
 */

#include "storage/hash/siphash.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

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

static inline uint64_t
read64le(const uint8_t *p)
{
	return (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16)
	       | ((uint64_t)p[3] << 24) | ((uint64_t)p[4] << 32)
	       | ((uint64_t)p[5] << 40) | ((uint64_t)p[6] << 48)
	       | ((uint64_t)p[7] << 56);
}

static uint64_t global_k0 = 0;
static uint64_t global_k1 = 0;
static int keys_initialized = 0;

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
			return 0;
		}
	}

	srand((unsigned int)time(NULL) ^ getpid());
	*k0 = ((uint64_t)rand() << 32) | (uint64_t)rand();
	*k1 = ((uint64_t)rand() << 32) | (uint64_t)rand();
	return -1;
}

void
siphash_set_key(uint64_t k0, uint64_t k1)
{
	global_k0 = k0;
	global_k1 = k1;
	keys_initialized = 1;
}

uint64_t
siphash_with_global_key(const void *data, size_t len)
{
	if (!keys_initialized) {
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

	v0 = 0x736f6d6570736575ULL ^ k0;
	v1 = 0x646f72616e646f6dULL ^ k1;
	v2 = 0x6c7967656e657261ULL ^ k0;
	v3 = 0x7465646279746573ULL ^ k1;

	end = in + len - (len % 8);
	for (; in != end; in += 8) {
		m = read64le(in);
		v3 ^= m;
		SIPROUND;
		SIPROUND;
		v0 ^= m;
	}

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
	SIPROUND;
	v0 ^= b;

	v2 ^= 0xff;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	SIPROUND;

	return v0 ^ v1 ^ v2 ^ v3;
}

uint64_t
siphash_key(const void *data, size_t len, const uint8_t key[16])
{
	uint64_t k0 = read64le(key);
	uint64_t k1 = read64le(key + 8);
	return siphash(data, len, k0, k1);
}
