/*
 * Hash Table Storage Engine - Core Implementation
 *
 * ARM-optimized kernel hash table with collision resolution for
 * high-performance storage operations via /dev/storage-hash.
 */

#include <linux/atomic.h>
#include <linux/errno.h>
#include <linux/hash.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>

#define HASH_BUCKET_COUNT_DEFAULT 1024
#define HASH_MAX_KEY_SIZE 256
#define HASH_MAX_VALUE_SIZE 4096

/*
 * Hash bucket entry - single key-value pair with chaining for collisions
 */
struct hash_entry {
	void *key;
	size_t key_len;
	void *value;
	size_t value_len;
	struct hash_entry *next;
};

/*
 * Hash bucket - head of collision chain
 */
struct hash_bucket {
	struct hash_entry *head;
	spinlock_t lock;
};

/*
 * Main hash engine structure
 */
struct hash_engine {
	struct hash_bucket *buckets;
	uint32_t bucket_count;
	spinlock_t engine_lock;
	atomic_t item_count;
	atomic_t total_memory;
};

static struct hash_engine *global_engine = NULL;

/*
 * Simple hash function - will be optimized with ARM NEON later
 */
static uint32_t
hash_function(const void *key, size_t key_len, uint32_t bucket_count)
{
	uint32_t hash = 0;
	const unsigned char *data = (const unsigned char *)key;
	size_t i;

	for (i = 0; i < key_len; i++) {
		hash = hash * 31 + data[i];
	}

	return hash % bucket_count;
}

/*
 * Compare two keys for equality
 */
static int
key_compare(const void *key1, size_t key1_len, const void *key2,
	    size_t key2_len)
{
	if (key1_len != key2_len)
		return 1;
	return memcmp(key1, key2, key1_len);
}

/*
 * Create a new hash entry
 */
static struct hash_entry *
hash_entry_create(const void *key, size_t key_len, const void *value,
		  size_t value_len)
{
	struct hash_entry *entry;

	if (key_len > HASH_MAX_KEY_SIZE || value_len > HASH_MAX_VALUE_SIZE)
		return NULL;

	entry = kmalloc(sizeof(struct hash_entry), GFP_KERNEL);
	if (!entry)
		return NULL;

	entry->key = kmalloc(key_len, GFP_KERNEL);
	if (!entry->key) {
		kfree(entry);
		return NULL;
	}

	entry->value = kmalloc(value_len, GFP_KERNEL);
	if (!entry->value) {
		kfree(entry->key);
		kfree(entry);
		return NULL;
	}

	memcpy(entry->key, key, key_len);
	memcpy(entry->value, value, value_len);
	entry->key_len = key_len;
	entry->value_len = value_len;
	entry->next = NULL;

	return entry;
}

/*
 * Free a hash entry
 */
static void
hash_entry_destroy(struct hash_entry *entry)
{
	if (entry) {
		kfree(entry->key);
		kfree(entry->value);
		kfree(entry);
	}
}

/*
 * Initialize hash engine
 */
int
hash_engine_init(uint32_t bucket_count)
{
	uint32_t i;

	if (global_engine)
		return -EEXIST;

	if (bucket_count == 0)
		bucket_count = HASH_BUCKET_COUNT_DEFAULT;

	global_engine = kmalloc(sizeof(struct hash_engine), GFP_KERNEL);
	if (!global_engine)
		return -ENOMEM;

	global_engine->buckets
	    = kmalloc(bucket_count * sizeof(struct hash_bucket), GFP_KERNEL);
	if (!global_engine->buckets) {
		kfree(global_engine);
		global_engine = NULL;
		return -ENOMEM;
	}

	global_engine->bucket_count = bucket_count;
	spin_lock_init(&global_engine->engine_lock);
	atomic_set(&global_engine->item_count, 0);
	atomic_set(&global_engine->total_memory, 0);

	for (i = 0; i < bucket_count; i++) {
		global_engine->buckets[i].head = NULL;
		spin_lock_init(&global_engine->buckets[i].lock);
	}

	printk(KERN_INFO "hash_engine: Initialized with %u buckets\n",
	       bucket_count);
	return 0;
}

/*
 * Cleanup hash engine
 */
void
hash_engine_cleanup(void)
{
	uint32_t i;
	struct hash_entry *entry, *next;

	if (!global_engine)
		return;

	for (i = 0; i < global_engine->bucket_count; i++) {
		spin_lock(&global_engine->buckets[i].lock);
		entry = global_engine->buckets[i].head;
		while (entry) {
			next = entry->next;
			hash_entry_destroy(entry);
			entry = next;
		}
		spin_unlock(&global_engine->buckets[i].lock);
	}

	kfree(global_engine->buckets);
	kfree(global_engine);
	global_engine = NULL;

	printk(KERN_INFO "hash_engine: Cleaned up\n");
}

/*
 * Put key-value pair into hash table
 */
int
hash_put(const void *key, size_t key_len, const void *value, size_t value_len)
{
	uint32_t bucket_idx;
	struct hash_bucket *bucket;
	struct hash_entry *entry, *new_entry;
	unsigned long flags;

	if (!global_engine)
		return -ENODEV;

	if (!key || key_len == 0 || !value || value_len == 0)
		return -EINVAL;

	bucket_idx = hash_function(key, key_len, global_engine->bucket_count);
	bucket = &global_engine->buckets[bucket_idx];

	new_entry = hash_entry_create(key, key_len, value, value_len);
	if (!new_entry)
		return -ENOMEM;

	spin_lock_irqsave(&bucket->lock, flags);

	/* Check if key already exists and update */
	entry = bucket->head;
	while (entry) {
		if (key_compare(entry->key, entry->key_len, key, key_len)
		    == 0) {
			/* Update existing entry */
			kfree(entry->value);
			entry->value = kmalloc(value_len, GFP_ATOMIC);
			if (!entry->value) {
				spin_unlock_irqrestore(&bucket->lock, flags);
				hash_entry_destroy(new_entry);
				return -ENOMEM;
			}
			memcpy(entry->value, value, value_len);
			entry->value_len = value_len;
			spin_unlock_irqrestore(&bucket->lock, flags);
			hash_entry_destroy(new_entry);
			return 0;
		}
		entry = entry->next;
	}

	/* Add new entry at head of chain */
	new_entry->next = bucket->head;
	bucket->head = new_entry;
	atomic_inc(&global_engine->item_count);
	atomic_add(key_len + value_len + sizeof(struct hash_entry),
		   &global_engine->total_memory);

	spin_unlock_irqrestore(&bucket->lock, flags);
	return 0;
}

/*
 * Get value by key from hash table
 */
int
hash_get(const void *key, size_t key_len, void **value, size_t *value_len)
{
	uint32_t bucket_idx;
	struct hash_bucket *bucket;
	struct hash_entry *entry;
	unsigned long flags;
	void *result_value = NULL;

	if (!global_engine)
		return -ENODEV;

	if (!key || key_len == 0 || !value || !value_len)
		return -EINVAL;

	bucket_idx = hash_function(key, key_len, global_engine->bucket_count);
	bucket = &global_engine->buckets[bucket_idx];

	spin_lock_irqsave(&bucket->lock, flags);

	entry = bucket->head;
	while (entry) {
		if (key_compare(entry->key, entry->key_len, key, key_len)
		    == 0) {
			/* Found the key - copy value */
			result_value = kmalloc(entry->value_len, GFP_ATOMIC);
			if (!result_value) {
				spin_unlock_irqrestore(&bucket->lock, flags);
				return -ENOMEM;
			}
			memcpy(result_value, entry->value, entry->value_len);
			*value = result_value;
			*value_len = entry->value_len;
			spin_unlock_irqrestore(&bucket->lock, flags);
			return 0;
		}
		entry = entry->next;
	}

	spin_unlock_irqrestore(&bucket->lock, flags);
	return -ENOENT;
}

/*
 * Delete key-value pair from hash table
 */
int
hash_delete(const void *key, size_t key_len)
{
	uint32_t bucket_idx;
	struct hash_bucket *bucket;
	struct hash_entry *entry, *prev;
	unsigned long flags;

	if (!global_engine)
		return -ENODEV;

	if (!key || key_len == 0)
		return -EINVAL;

	bucket_idx = hash_function(key, key_len, global_engine->bucket_count);
	bucket = &global_engine->buckets[bucket_idx];

	spin_lock_irqsave(&bucket->lock, flags);

	prev = NULL;
	entry = bucket->head;
	while (entry) {
		if (key_compare(entry->key, entry->key_len, key, key_len)
		    == 0) {
			/* Found the key - remove it */
			if (prev)
				prev->next = entry->next;
			else
				bucket->head = entry->next;

			atomic_dec(&global_engine->item_count);
			atomic_sub(entry->key_len + entry->value_len
				       + sizeof(struct hash_entry),
				   &global_engine->total_memory);

			spin_unlock_irqrestore(&bucket->lock, flags);
			hash_entry_destroy(entry);
			return 0;
		}
		prev = entry;
		entry = entry->next;
	}

	spin_unlock_irqrestore(&bucket->lock, flags);
	return -ENOENT;
}

/*
 * Get hash engine statistics
 */
int
hash_get_stats(uint32_t *item_count, uint32_t *bucket_count,
	       uint32_t *memory_usage)
{
	if (!global_engine)
		return -ENODEV;

	if (item_count)
		*item_count = atomic_read(&global_engine->item_count);
	if (bucket_count)
		*bucket_count = global_engine->bucket_count;
	if (memory_usage)
		*memory_usage = atomic_read(&global_engine->total_memory);

	return 0;
}

EXPORT_SYMBOL(hash_engine_init);
EXPORT_SYMBOL(hash_engine_cleanup);
EXPORT_SYMBOL(hash_put);
EXPORT_SYMBOL(hash_get);
EXPORT_SYMBOL(hash_delete);
EXPORT_SYMBOL(hash_get_stats);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ARM Storage Engine Learning Project");
MODULE_DESCRIPTION("Kernel Hash Table Storage Engine");
MODULE_VERSION("1.0");