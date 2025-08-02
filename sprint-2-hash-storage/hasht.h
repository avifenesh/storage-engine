#ifndef HASHT_H
#define HASHT_H

#define MAX_LOAD_FACTOR 0.7 // maximum load factor before resizing
#define MIN_LOAD_FACTOR 0.2 // minimum load factor before resizing

// hash table implementation with linear probing
struct HashTable {
	int size;
	int count;
	char **keys;
	void **values;
};

void hasht_init(struct HashTable *ht, int size);

void hasht_free(struct HashTable *ht);

void hasht_insert(struct HashTable *ht, const char *key, void *value);

void hasht_remove(struct HashTable *ht, const char *key);

void *hasht_get(struct HashTable *ht, const char *key);

int hasht_hash(const char *key);

void resize_shrink(struct HashTable *ht);

void resize_grow(struct HashTable *ht);

#endif
