/**
 * @file btree_engine.h
 * @brief Public API for a userspace B+ tree engine (scaffold).
 */

#ifndef BTREE_ENGINE_H
#define BTREE_ENGINE_H

#include <stddef.h>
#include <stdint.h>

struct btree_engine {
	void *root;
	uint32_t height;
	uint32_t node_count;
};

int btree_engine_init(struct btree_engine *tree);
int btree_engine_destroy(struct btree_engine *tree);

int btree_insert(struct btree_engine *tree, const void *key, size_t key_len,
		const void *value, size_t value_len);
int btree_search(struct btree_engine *tree, const void *key, size_t key_len,
		const void **value, size_t *value_len);
int btree_delete(struct btree_engine *tree, const void *key, size_t key_len);

#endif /* BTREE_ENGINE_H */

