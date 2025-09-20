#include <stddef.h>
#include <stdint.h>

// Userspace-friendly stub. Replace with NEON under __aarch64__.
int
neon_key_compare(const void *key1, size_t len1, const void *key2, size_t len2)
{
	const unsigned char *a = (const unsigned char *)key1;
	const unsigned char *b = (const unsigned char *)key2;
	size_t n = (len1 < len2) ? len1 : len2;
	for (size_t i = 0; i < n; i++) {
		if (a[i] != b[i])
			return (int)a[i] - (int)b[i];
	}
	return (len1 < len2) ? -1 : (len1 > len2) ? 1 : 0;
}
