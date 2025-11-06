/**
 * @file btree_engine.c
 * @brief Scaffold for userspace B+ tree engine.
 */

#include "storage/btree_engine.h"
#include <errno.h>
#include <string.h>

int
btree_engine_init(struct btree_engine *tree)
{
	if (!tree)
		return -EINVAL;
	memset(tree, 0, sizeof(*tree));
	return 0;
}

int
btree_engine_destroy(struct btree_engine *tree)
{
	if (!tree)
		return -EINVAL;
	return 0;
}

int
btree_insert(struct btree_engine *tree, const void *key, size_t key_len,
	    const void *value, size_t value_len)
{
	(void)tree;
	(void)key;
	(void)key_len;
	(void)value;
	(void)value_len;
	return -ENOSYS;
}

int
btree_search(struct btree_engine *tree, const void *key, size_t key_len,
	    const void **value, size_t *value_len)
{
	(void)tree;
	(void)key;
	(void)key_len;
	(void)value;
	(void)value_len;
	return -ENOSYS;
}

int
btree_delete(struct btree_engine *tree, const void *key, size_t key_len)
{
	(void)tree;
	(void)key;
	(void)key_len;
	return -ENOSYS;
}

