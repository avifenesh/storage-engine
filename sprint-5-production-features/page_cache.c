#include "page_cache.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef PAGE_SIZE_BYTES
#define PAGE_SIZE_BYTES 4096
#endif

typedef struct pc_node {
    pc_page_t page;
    struct pc_node *prev, *next; // simple LRU list
} pc_node_t;

struct pcache {
    size_t capacity;
    size_t size;
    pc_node_t *lru_head; // most recently used
    pc_node_t *lru_tail; // least recently used
    // naive open-addressing hash table for page_id -> node
    size_t table_size;
    pc_node_t **table;
};

static size_t hash64(uint64_t x, size_t n) { x ^= x >> 33; x *= 0xff51afd7ed558ccdULL; x ^= x >> 33; x *= 0xc4ceb9fe1a85ec53ULL; x ^= x >> 33; return (size_t)(x & (n - 1)); }

static pc_node_t **find_slot(pcache_t *pc, uint64_t page_id)
{
    size_t idx = hash64(page_id, pc->table_size);
    for (size_t i = 0; i < pc->table_size; i++) {
        size_t p = (idx + i) & (pc->table_size - 1);
        pc_node_t **slot = &pc->table[p];
        if (!*slot || (*slot && (*slot)->page.page_id == page_id)) return slot;
    }
    return NULL;
}

static void lru_move_front(pcache_t *pc, pc_node_t *node)
{
    if (pc->lru_head == node) return;
    // detach
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    if (pc->lru_tail == node) pc->lru_tail = node->prev;
    // push front
    node->prev = NULL; node->next = pc->lru_head;
    if (pc->lru_head) pc->lru_head->prev = node;
    pc->lru_head = node;
    if (!pc->lru_tail) pc->lru_tail = node;
}

static void lru_push_front(pcache_t *pc, pc_node_t *node)
{
    node->prev = NULL; node->next = pc->lru_head;
    if (pc->lru_head) pc->lru_head->prev = node;
    pc->lru_head = node;
    if (!pc->lru_tail) pc->lru_tail = node;
}

pcache_t *pc_create(size_t capacity_pages)
{
    pcache_t *pc;
    size_t ts;
    if (capacity_pages == 0) capacity_pages = 64;
    pc = (pcache_t *)calloc(1, sizeof(*pc));
    if (!pc) return NULL;
    pc->capacity = capacity_pages;
    // power-of-two hash table size at least 2x capacity
    ts = 1; while (ts < (capacity_pages << 1)) ts <<= 1;
    pc->table_size = ts;
    pc->table = (pc_node_t **)calloc(pc->table_size, sizeof(pc_node_t *));
    if (!pc->table) { free(pc); return NULL; }
    return pc;
}

void pc_destroy(pcache_t *pc)
{
    if (!pc) return;
    for (size_t i = 0; i < pc->table_size; i++) {
        pc_node_t *n = pc->table[i];
        if (n) { free(n->page.data); free(n); }
    }
    free(pc->table);
    free(pc);
}

pc_page_t *pc_get(pcache_t *pc, uint64_t page_id)
{
    pc_node_t **slot;
    pc_node_t *node;
    if (!pc) return NULL;
    slot = find_slot(pc, page_id);
    if (!slot) return NULL;
    if (*slot) {
        lru_move_front(pc, *slot);
        return &(*slot)->page;
    }
    // miss
    node = (pc_node_t *)calloc(1, sizeof(*node));
    if (!node) return NULL;
    node->page.page_id = page_id;
    node->page.data = calloc(1, PAGE_SIZE_BYTES);
    if (!node->page.data) { free(node); return NULL; }
    *slot = node;
    lru_push_front(pc, node);
    pc->size++;
    // if over capacity, evict LRU (not the one we just inserted).
    if (pc->size > pc->capacity) pc_evict_one(pc);
    return &node->page;
}

void pc_pin(pcache_t *pc, pc_page_t *page)
{ (void)pc; if (page) page->pins++; }

void pc_unpin(pcache_t *pc, pc_page_t *page)
{ (void)pc; if (page && page->pins > 0) page->pins--; }

void pc_mark_dirty(pcache_t *pc, pc_page_t *page)
{ (void)pc; if (page) page->dirty = true; }

int pc_evict_one(pcache_t *pc)
{
    pc_node_t *n;
    pc_node_t **slot;
    if (!pc || !pc->lru_tail) return -ENOENT;
    n = pc->lru_tail;
    // skip pinned pages; walk backwards
    while (n && n->page.pins > 0) n = n->prev;
    if (!n) return -EBUSY;
    // unlink from LRU
    if (n->prev) n->prev->next = n->next;
    if (n->next) n->next->prev = n->prev;
    if (pc->lru_head == n) pc->lru_head = n->next;
    if (pc->lru_tail == n) pc->lru_tail = n->prev;
    // remove from hash
    slot = find_slot(pc, n->page.page_id);
    if (slot && *slot == n) *slot = NULL;
    free(n->page.data);
    free(n);
    pc->size--;
    return 0;
}

int pc_flush_dirty(pcache_t *pc, pc_flush_cb cb, void *ctx)
{
    if (!pc || !cb) return -EINVAL;
    // naive scan of hash table
    for (size_t i = 0; i < pc->table_size; i++) {
        pc_node_t *n = pc->table[i];
        if (n && n->page.dirty) {
            int rc = cb(&n->page, ctx);
            if (rc != 0) return rc;
            n->page.dirty = false;
        }
    }
    return 0;
}
