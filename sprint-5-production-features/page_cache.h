#ifndef SPRINT5_PAGE_CACHE_H
#define SPRINT5_PAGE_CACHE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct pcache pcache_t;

typedef struct {
	uint64_t page_id;
	void *data; // PAGE_SIZE buffer
	bool dirty;
	int pins; // pin count
} pc_page_t;

pcache_t *pc_create(size_t capacity_pages);
void pc_destroy(pcache_t *pc);

// Get a page; if not present, allocate and return zeroed page.
pc_page_t *pc_get(pcache_t *pc, uint64_t page_id);
void pc_pin(pcache_t *pc, pc_page_t *page);
void pc_unpin(pcache_t *pc, pc_page_t *page);
void pc_mark_dirty(pcache_t *pc, pc_page_t *page);

// Evict one page (clock/LRU policy). Returns 0 on success, <0 if none evicted.
int pc_evict_one(pcache_t *pc);

// Iterate all pages to flush dirty ones (callback can perform I/O).
typedef int (*pc_flush_cb)(const pc_page_t *page, void *ctx);
int pc_flush_dirty(pcache_t *pc, pc_flush_cb cb, void *ctx);

#endif // SPRINT5_PAGE_CACHE_H
