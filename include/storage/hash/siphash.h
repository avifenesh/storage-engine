#ifndef SIPHASH_H
#define SIPHASH_H

#include <stddef.h>
#include <stdint.h>

uint64_t siphash(const void *data, size_t len, uint64_t k0, uint64_t k1);
uint64_t siphash_key(const void *data, size_t len, const uint8_t key[16]);
int siphash_init_random_key(uint64_t *k0, uint64_t *k1);
void siphash_set_key(uint64_t k0, uint64_t k1);
uint64_t siphash_with_global_key(const void *data, size_t len);

#endif
