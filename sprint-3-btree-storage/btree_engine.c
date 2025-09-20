#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Minimal stubs matching docs. Flesh out in userspace first.

struct btree_node;

struct btree_engine {
	struct btree_node *root;
	uint32_t height;
};

int
btree_insert(struct btree_engine *tree, const void *key, size_t key_len,
	     const void *value, size_t value_len)
{
	(void)tree;
	(void)key;
	(void)key_len;
	(void)value;
	(void)value_len;
	return -1; // TODO: implement userspace prototype
}

int
btree_search(struct btree_engine *tree, const void *key, size_t key_len,
	     void **value, size_t *value_len)
{
	(void)tree;
	(void)key;
	(void)key_len;
	(void)value;
	(void)value_len;
	return -1; // TODO
}

int
btree_delete(struct btree_engine *tree, const void *key, size_t key_len)
{
	(void)tree;
	(void)key;
	(void)key_len;
	return -1; // TODO
}
