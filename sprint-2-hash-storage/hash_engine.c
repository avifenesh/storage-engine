#include <stddef.h>
#include <stdint.h>

// Minimal userspace prototype API. Expand per docs.

int
hash_put(const void *key, size_t key_len, const void *value, size_t value_len)
{
	(void)key;
	(void)key_len;
	(void)value;
	(void)value_len;
	return -1; // TODO: implement
}

int
hash_get(const void *key, size_t key_len, void **value, size_t *value_len)
{
	(void)key;
	(void)key_len;
	(void)value;
	(void)value_len;
	return -1; // TODO
}

int
hash_delete(const void *key, size_t key_len)
{
	(void)key;
	(void)key_len;
	return -1; // TODO
}

int
hash_get_stats(uint32_t *item_count, uint32_t *bucket_count,
	       uint32_t *memory_usage)
{
	if (item_count)
		*item_count = 0;
	if (bucket_count)
		*bucket_count = 0;
	if (memory_usage)
		*memory_usage = 0;
	return 0;
}
