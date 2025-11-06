#include <stddef.h>
#include <stdint.h>

uint32_t
neon_hash_function(const void *key, size_t len)
{
	const unsigned char *p = (const unsigned char *)key;
	uint32_t h = 2166136261u;
	for (size_t i = 0; i < len; i++) {
		h ^= p[i];
		h *= 16777619u;
	}
	return h;
}

int
neon_key_compare(const void *key1, size_t key1_len, const void *key2,
		 size_t key2_len)
{
	const unsigned char *a = (const unsigned char *)key1;
	const unsigned char *b = (const unsigned char *)key2;
	size_t n = (key1_len < key2_len) ? key1_len : key2_len;
	for (size_t i = 0; i < n; i++) {
		if (a[i] != b[i])
			return (int)a[i] - (int)b[i];
	}
	return (key1_len < key2_len) ? -1 : (key1_len > key2_len) ? 1 : 0;
}

