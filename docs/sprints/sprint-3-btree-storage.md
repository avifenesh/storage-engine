# Sprint 3: B+ Tree Storage Engine

## Overview
Build a kernel-space B+ tree storage engine for sorted data storage and range queries. Create `/dev/storage-btree` character device with optimized tree operations and ARM NEON key comparison.

**Duration**: 5 weeks  
**Prerequisites**: Completed Sprint 2 (hash table storage engine)

---

## 🎯 Learning Objectives

1. **Master B+ tree algorithms** - Implement insertion, deletion, and range queries
2. **Design sorted storage** - Efficient storage for ordered data access
3. **Optimize with ARM NEON** - SIMD key comparison and data movement
4. **Handle complex operations** - Node splitting, merging, and rebalancing
5. **Implement range queries** - Efficient sequential access patterns

---

## 📋 Sprint Issues

### Issue #31: Implement Core B+ Tree Data Structure in Kernel
**File**: `sprint-3-btree-storage/btree_engine.c`

**Requirements**:
- B+ tree with configurable order
- Leaf nodes linked for range queries
- Proper node splitting and merging
- Memory-efficient storage layout

**Core B+ Tree Structure**:
```c
#define BTREE_ORDER 64  // Optimized for cache lines
#define BTREE_MIN_KEYS (BTREE_ORDER / 2)
#define BTREE_MAX_KEYS (BTREE_ORDER - 1)

struct btree_node {
    bool is_leaf;
    uint16_t num_keys;
    uint16_t node_id;
    
    // Keys array - variable length data
    void **keys;
    size_t *key_lens;
    
    union {
        // For internal nodes
        struct {
            struct btree_node **children;
        } internal;
        
        // For leaf nodes
        struct {
            void **values;
            size_t *value_lens;
            struct btree_node *next_leaf;  // For range queries
        } leaf;
    };
    
    struct btree_node *parent;
    spinlock_t node_lock;
};

struct btree_engine {
    struct btree_node *root;
    uint32_t height;
    atomic_t total_keys;
    atomic_t total_nodes;
    rwlock_t tree_lock;
    
    // Memory management
    struct kmem_cache *node_cache;
    struct kmem_cache *key_cache;
};

// Core operations
int btree_insert(struct btree_engine *tree, const void *key, size_t key_len,
                const void *value, size_t value_len);
int btree_search(struct btree_engine *tree, const void *key, size_t key_len,
                void **value, size_t *value_len);
int btree_delete(struct btree_engine *tree, const void *key, size_t key_len);
int btree_range_query(struct btree_engine *tree, const void *start_key, size_t start_len,
                     const void *end_key, size_t end_len, 
                     struct btree_range_result *results);
```

**Node Operations**:
```c
// Node creation and management
struct btree_node *btree_create_node(struct btree_engine *tree, bool is_leaf);
void btree_destroy_node(struct btree_engine *tree, struct btree_node *node);

// Node splitting when full
int btree_split_internal_node(struct btree_engine *tree, struct btree_node *node);
int btree_split_leaf_node(struct btree_engine *tree, struct btree_node *node);

// Node merging when underflow
int btree_merge_nodes(struct btree_engine *tree, struct btree_node *left, 
                     struct btree_node *right);

// Key operations within nodes
int btree_insert_key_in_node(struct btree_node *node, const void *key, size_t key_len,
                             const void *value, size_t value_len, int position);
int btree_remove_key_from_node(struct btree_node *node, int position);
```

**Success Criteria**:
- O(log n) search, insert, delete operations
- Maintains B+ tree invariants (balanced, sorted)
- Efficient memory usage with node caching
- Handles concurrent access safely

---

### Issue #32: Build Character Device Interface for B+ Tree Operations
**File**: `sprint-3-btree-storage/btree_device.c`

**Requirements**:
- Character device `/dev/storage-btree`
- IOCTL commands for tree operations
- Range query support
- Iterator interface for large result sets

**IOCTL Interface**:
```c
#define BTREE_IOC_MAGIC 'B'
#define BTREE_IOC_INSERT    _IOW(BTREE_IOC_MAGIC, 1, struct btree_operation)
#define BTREE_IOC_SEARCH    _IOWR(BTREE_IOC_MAGIC, 2, struct btree_operation)
#define BTREE_IOC_DELETE    _IOW(BTREE_IOC_MAGIC, 3, struct btree_operation)
#define BTREE_IOC_RANGE     _IOWR(BTREE_IOC_MAGIC, 4, struct btree_range_query)
#define BTREE_IOC_STATS     _IOR(BTREE_IOC_MAGIC, 5, struct btree_stats)
#define BTREE_IOC_ITERATOR  _IOWR(BTREE_IOC_MAGIC, 6, struct btree_iterator)

struct btree_operation {
    void __user *key;
    size_t key_len;
    void __user *value;
    size_t value_len;
};

struct btree_range_query {
    void __user *start_key;
    size_t start_key_len;
    void __user *end_key;
    size_t end_key_len;
    uint32_t max_results;
    
    // Results
    void __user *keys_buffer;
    void __user *values_buffer; 
    uint32_t *result_count;
};

struct btree_stats {
    uint32_t total_keys;
    uint32_t total_nodes;
    uint32_t tree_height;
    uint64_t memory_usage;
    uint32_t cache_hits;
    uint32_t cache_misses;
};

struct btree_iterator {
    uint64_t iterator_id;
    void __user *key_buffer;
    void __user *value_buffer;
    size_t buffer_size;
    bool has_next;
};
```

**Range Query Implementation**:
```c
// Efficient range query using leaf node traversal
static long handle_range_query(struct btree_range_query __user *arg) {
    struct btree_range_query query;
    struct btree_range_result *results;
    struct btree_node *start_leaf;
    int ret = 0;
    
    if (copy_from_user(&query, arg, sizeof(query)))
        return -EFAULT;
    
    // Find starting leaf node
    start_leaf = btree_find_leaf_node(global_btree, query.start_key, 
                                     query.start_key_len);
    if (!start_leaf)
        return -ENOENT;
    
    // Allocate result buffer
    results = kzalloc(sizeof(*results) + 
                     query.max_results * sizeof(struct btree_kv_pair),
                     GFP_KERNEL);
    if (!results)
        return -ENOMEM;
    
    // Traverse leaf nodes collecting results
    ret = btree_collect_range_results(start_leaf, &query, results);
    if (ret < 0)
        goto cleanup;
    
    // Copy results to user space
    ret = copy_range_results_to_user(&query, results);
    
cleanup:
    kfree(results);
    return ret;
}
```

**Success Criteria**:
- All IOCTL operations work correctly
- Range queries handle large result sets efficiently
- Iterator interface supports streaming large datasets
- Proper error handling and resource cleanup

---

### Issue #33: ARM NEON Key Comparison Optimization
**File**: `sprint-3-btree-storage/neon_compare.c`

**Requirements**:
- SIMD-optimized key comparison functions
- Variable-length key support
- Batch comparison for node operations
- Performance improvements over scalar comparison

**NEON Key Comparison**:
```c
// SIMD string comparison optimized for B+ tree keys
int neon_key_compare(const void *key1, size_t len1, const void *key2, size_t len2) {
    const uint8_t *p1 = (const uint8_t *)key1;
    const uint8_t *p2 = (const uint8_t *)key2;
    size_t min_len = len1 < len2 ? len1 : len2;
    size_t i = 0;
    
#ifdef __aarch64__
    if (may_use_simd() && min_len >= 16) {
        kernel_neon_begin();
        
        // Process 16 bytes at a time
        for (; i + 16 <= min_len; i += 16) {
            uint8x16_t v1 = vld1q_u8(p1 + i);
            uint8x16_t v2 = vld1q_u8(p2 + i);
            uint8x16_t cmp = vceqq_u8(v1, v2);
            
            // Check if all bytes are equal
            uint64x2_t result = vreinterpretq_u64_u8(cmp);
            if (vgetq_lane_u64(result, 0) != 0xFFFFFFFFFFFFFFFFULL ||
                vgetq_lane_u64(result, 1) != 0xFFFFFFFFFFFFFFFFULL) {
                
                // Find first differing byte
                for (size_t j = 0; j < 16; j++) {
                    if (p1[i + j] != p2[i + j]) {
                        kernel_neon_end();
                        return (int)p1[i + j] - (int)p2[i + j];
                    }
                }
            }
        }
        
        kernel_neon_end();
    }
#endif
    
    // Handle remaining bytes
    for (; i < min_len; i++) {
        if (p1[i] != p2[i])
            return (int)p1[i] - (int)p2[i];
    }
    
    // All compared bytes equal, compare lengths
    return (len1 < len2) ? -1 : (len1 > len2) ? 1 : 0;
}

// Binary search in node with SIMD optimization
int neon_binary_search_node(struct btree_node *node, const void *key, size_t key_len) {
    int left = 0;
    int right = node->num_keys - 1;
    int result = -1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = neon_key_compare(key, key_len, 
                                  node->keys[mid], node->key_lens[mid]);
        
        if (cmp == 0) {
            result = mid;
            break;
        } else if (cmp < 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
            result = left;  // Insert position
        }
    }
    
    return result;
}

// Batch key comparison for node operations
void neon_sort_keys_in_node(struct btree_node *node) {
    // Optimized quicksort using SIMD comparisons
    if (node->num_keys <= 1)
        return;
    
#ifdef __aarch64__
    if (may_use_simd()) {
        kernel_neon_begin();
        neon_quicksort_keys(node->keys, node->key_lens, 0, node->num_keys - 1);
        kernel_neon_end();
        return;
    }
#endif
    
    // Fallback to scalar quicksort
    scalar_quicksort_keys(node->keys, node->key_lens, 0, node->num_keys - 1);
}
```

**Memory-Efficient Operations**:
```c
// SIMD memory operations for node data movement
void neon_move_keys_in_node(struct btree_node *node, int from_pos, int to_pos, 
                           int count) {
    if (count <= 0 || from_pos == to_pos)
        return;
    
#ifdef __aarch64__
    if (may_use_simd() && count >= 4) {
        kernel_neon_begin();
        
        // Use NEON for efficient memory movement
        size_t bytes_to_move = count * sizeof(void *);
        uint8_t *src = (uint8_t *)&node->keys[from_pos];
        uint8_t *dst = (uint8_t *)&node->keys[to_pos];
        
        // Copy 16 bytes at a time
        for (size_t i = 0; i + 16 <= bytes_to_move; i += 16) {
            uint8x16_t data = vld1q_u8(src + i);
            vst1q_u8(dst + i, data);
        }
        
        kernel_neon_end();
        return;
    }
#endif
    
    // Fallback to memmove
    memmove(&node->keys[to_pos], &node->keys[from_pos], 
            count * sizeof(void *));
    memmove(&node->key_lens[to_pos], &node->key_lens[from_pos],
            count * sizeof(size_t));
}
```

**Success Criteria**:
- 3x speedup for key comparisons on large datasets
- Maintains correct ordering with variable-length keys
- Efficient bulk operations for node management
- Graceful fallback on non-ARM64 systems

---

### Issue #34: Implement Range Queries and Sequential Access
**File**: `sprint-3-btree-storage/btree_range.c`

**Requirements**:
- Efficient range query implementation
- Sequential access through leaf nodes
- Large result set handling
- Iterator pattern for memory efficiency

**Range Query Implementation**:
```c
struct btree_range_result {
    struct btree_kv_pair *pairs;
    uint32_t count;
    uint32_t capacity;
    bool has_more;
};

struct btree_kv_pair {
    void *key;
    size_t key_len;
    void *value;
    size_t value_len;
};

// Efficient range query using leaf traversal
int btree_range_query(struct btree_engine *tree, const void *start_key, size_t start_len,
                     const void *end_key, size_t end_len, uint32_t max_results,
                     struct btree_range_result *results) {
    struct btree_node *current_leaf;
    int start_pos = 0;
    
    read_lock(&tree->tree_lock);
    
    // Find starting leaf node and position
    current_leaf = btree_find_leaf_for_key(tree, start_key, start_len);
    if (!current_leaf) {
        read_unlock(&tree->tree_lock);
        return -ENOENT;
    }
    
    // Find starting position in leaf
    start_pos = neon_binary_search_node(current_leaf, start_key, start_len);
    if (start_pos < 0) start_pos = 0;
    
    // Collect results across leaf nodes
    results->count = 0;
    results->has_more = false;
    
    while (current_leaf && results->count < max_results) {
        spin_lock(&current_leaf->node_lock);
        
        // Process keys in current leaf
        for (int i = start_pos; i < current_leaf->num_keys && 
             results->count < max_results; i++) {
            
            // Check if we've exceeded end key
            if (end_key && neon_key_compare(current_leaf->leaf.keys[i],
                                          current_leaf->key_lens[i],
                                          end_key, end_len) > 0) {
                spin_unlock(&current_leaf->node_lock);
                goto range_complete;
            }
            
            // Add to results
            if (add_to_range_results(results, current_leaf->leaf.keys[i],
                                   current_leaf->key_lens[i],
                                   current_leaf->leaf.values[i],
                                   current_leaf->leaf.value_lens[i]) < 0) {
                spin_unlock(&current_leaf->node_lock);
                read_unlock(&tree->tree_lock);
                return -ENOMEM;
            }
        }
        
        // Move to next leaf
        struct btree_node *next_leaf = current_leaf->leaf.next_leaf;
        spin_unlock(&current_leaf->node_lock);
        current_leaf = next_leaf;
        start_pos = 0;  // Start from beginning of next leaf
    }
    
    // Set has_more flag if we stopped due to max_results limit
    if (current_leaf && results->count >= max_results) {
        results->has_more = true;
    }
    
range_complete:
    read_unlock(&tree->tree_lock);
    return 0;
}
```

**Iterator Implementation**:
```c
struct btree_iterator {
    uint64_t id;
    struct btree_engine *tree;
    struct btree_node *current_leaf;
    int current_position;
    void *start_key;
    size_t start_key_len;
    void *end_key;
    size_t end_key_len;
    bool is_valid;
    spinlock_t iter_lock;
};

// Create iterator for range
struct btree_iterator *btree_create_iterator(struct btree_engine *tree,
                                           const void *start_key, size_t start_len,
                                           const void *end_key, size_t end_len) {
    struct btree_iterator *iter = kzalloc(sizeof(*iter), GFP_KERNEL);
    if (!iter)
        return NULL;
    
    iter->id = atomic64_inc_return(&tree->next_iterator_id);
    iter->tree = tree;
    iter->is_valid = true;
    spin_lock_init(&iter->iter_lock);
    
    // Copy key bounds
    if (start_key && start_len > 0) {
        iter->start_key = kmalloc(start_len, GFP_KERNEL);
        if (iter->start_key) {
            memcpy(iter->start_key, start_key, start_len);
            iter->start_key_len = start_len;
        }
    }
    
    if (end_key && end_len > 0) {
        iter->end_key = kmalloc(end_len, GFP_KERNEL);
        if (iter->end_key) {
            memcpy(iter->end_key, end_key, end_len);
            iter->end_key_len = end_len;
        }
    }
    
    // Position iterator at start
    btree_iterator_seek_to_start(iter);
    
    return iter;
}

// Get next key-value pair from iterator
int btree_iterator_next(struct btree_iterator *iter, void **key, size_t *key_len,
                       void **value, size_t *value_len) {
    if (!iter || !iter->is_valid)
        return -EINVAL;
    
    spin_lock(&iter->iter_lock);
    
    if (!iter->current_leaf || iter->current_position >= iter->current_leaf->num_keys) {
        // Move to next leaf
        if (btree_iterator_advance_leaf(iter) < 0) {
            spin_unlock(&iter->iter_lock);
            return -ENOENT;  // End of iteration
        }
    }
    
    // Get current key-value pair
    *key = iter->current_leaf->leaf.keys[iter->current_position];
    *key_len = iter->current_leaf->key_lens[iter->current_position];
    *value = iter->current_leaf->leaf.values[iter->current_position];
    *value_len = iter->current_leaf->leaf.value_lens[iter->current_position];
    
    // Advance position
    iter->current_position++;
    
    spin_unlock(&iter->iter_lock);
    return 0;
}
```

**Success Criteria**:
- Range queries complete in O(log n + k) time (k = result count)
- Iterators handle large datasets without memory issues
- Sequential access maintains cache locality
- Proper cleanup of iterator resources

---

### Issue #35: Performance Testing and Optimization
**File**: `sprint-3-btree-storage/btree_benchmarks.c`

**Requirements**:
- Comprehensive benchmark suite
- Comparison with hash table from Sprint 2  
- Range query performance analysis
- Memory usage profiling

**Benchmark Framework**:
```c
// B+ tree specific benchmarks
typedef struct {
    const char *name;
    int (*setup_func)(struct btree_engine **tree);
    int (*benchmark_func)(struct btree_engine *tree, void *test_data);
    void (*cleanup_func)(struct btree_engine *tree);
    void *test_data;
    int iterations;
} btree_benchmark_t;

// Insert performance test
int benchmark_btree_insert(struct btree_engine *tree, void *test_data) {
    insert_test_data_t *data = (insert_test_data_t *)test_data;
    
    for (int i = 0; i < data->num_keys; i++) {
        char key[32], value[64];
        snprintf(key, sizeof(key), "key_%08d", i);
        snprintf(value, sizeof(value), "value_%08d_with_extra_data", i);
        
        int ret = btree_insert(tree, key, strlen(key), value, strlen(value));
        if (ret < 0)
            return ret;
    }
    
    return 0;
}

// Range query performance test
int benchmark_btree_range_query(struct btree_engine *tree, void *test_data) {
    range_test_data_t *data = (range_test_data_t *)test_data;
    struct btree_range_result results = {0};
    
    // Allocate result buffer
    results.capacity = data->max_results;
    results.pairs = kzalloc(results.capacity * sizeof(struct btree_kv_pair),
                          GFP_KERNEL);
    if (!results.pairs)
        return -ENOMEM;
    
    for (int i = 0; i < data->num_queries; i++) {
        char start_key[32], end_key[32];
        snprintf(start_key, sizeof(start_key), "key_%08d", i * 100);
        snprintf(end_key, sizeof(end_key), "key_%08d", i * 100 + 50);
        
        results.count = 0;
        int ret = btree_range_query(tree, start_key, strlen(start_key),
                                  end_key, strlen(end_key),
                                  data->max_results, &results);
        if (ret < 0) {
            kfree(results.pairs);
            return ret;
        }
    }
    
    kfree(results.pairs);
    return 0;
}

// Compare B+ tree vs Hash table performance
void compare_storage_engines(void) {
    const int test_size = 100000;
    
    printf("\n=== Storage Engine Comparison ===\n");
    printf("Test size: %d operations\n\n", test_size);
    
    // Test insert performance
    printf("%-20s %-15s %-15s %-10s\n", 
           "Operation", "Hash Table", "B+ Tree", "Ratio");
    printf("------------------------------------------------------------\n");
    
    // Insert benchmark
    double hash_insert_time = benchmark_hash_inserts(test_size);
    double btree_insert_time = benchmark_btree_inserts(test_size);
    
    printf("%-20s %-15.2f %-15.2f %.2fx\n", "Insert (ms)", 
           hash_insert_time, btree_insert_time, 
           btree_insert_time / hash_insert_time);
    
    // Search benchmark  
    double hash_search_time = benchmark_hash_searches(test_size);
    double btree_search_time = benchmark_btree_searches(test_size);
    
    printf("%-20s %-15.2f %-15.2f %.2fx\n", "Search (ms)",
           hash_search_time, btree_search_time,
           btree_search_time / hash_search_time);
    
    // Range query (B+ tree only)
    double btree_range_time = benchmark_btree_range_queries(test_size / 100);
    
    printf("%-20s %-15s %-15.2f %s\n", "Range Query (ms)",
           "N/A", btree_range_time, "B+ tree only");
}
```

**Success Criteria**:
- B+ tree search within 2x of hash table performance
- Range queries complete in <50ms for 1000 results
- Memory usage stays within 200% of raw data size
- Performance scales logarithmically with data size

---

## 🛠️ Implementation Guide

### Week 1: Core B+ Tree Structure
1. Design B+ tree node layout
2. Implement basic tree operations
3. Add node splitting and merging
4. Test with single-threaded operations

### Week 2: Character Device Interface
1. Create kernel module with B+ tree integration
2. Implement IOCTL command handlers
3. Add proper error handling and validation
4. Test device operations

### Week 3: ARM NEON Optimization
1. Implement SIMD key comparison
2. Optimize node operations with NEON
3. Add batch processing functions
4. Benchmark performance improvements

### Week 4: Range Queries and Iterators
1. Implement efficient range query algorithm
2. Build iterator interface
3. Add sequential access optimizations
4. Test with large datasets

### Week 5: Testing and Performance Analysis
1. Create comprehensive benchmark suite
2. Compare with hash table implementation
3. Profile memory usage and optimize
4. Document performance characteristics

---

## 📚 Resources

### B+ Tree Theory and Implementation
- [B+ Tree Visualization](https://www.cs.usfca.edu/~galles/visualization/BPlusTree.html) - Interactive B+ tree operations
- [Database Internals - B+ Trees](https://www.databass.dev/) - Chapter on B+ tree implementation
- [CMU Database Course](https://15445.courses.cs.cmu.edu/fall2021/notes/06-hashtables.pdf) - B+ tree lectures with examples
- [Build a Database Tutorial](https://cstack.github.io/db_tutorial/) - Step-by-step B+ tree implementation

### Linux Kernel Programming
- [The Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/) - Updated for modern kernels
- [IOCTL Tutorial](https://embetronicx.com/tutorials/linux/device-drivers/ioctl-tutorial-in-linux/) - Complete IOCTL implementation guide
- [Kernel Memory Management](https://www.kernel.org/doc/html/latest/core-api/memory-allocation.html) - kmalloc and caching

### ARM NEON Optimization
- [ARM NEON Programmer's Guide](https://developer.arm.com/documentation/102474/latest/) - Official ARM documentation
- [NEON Intrinsics Reference](https://arm-software.github.io/acle/neon_intrinsics/advsimd.html) - Complete intrinsics list
- [Vectorized String Comparison](https://arxiv.org/abs/1612.01506) - Research on SIMD string operations

### Video Resources
- Search "B+ tree implementation tutorial" on YouTube for visual explanations
- ARM Developer YouTube channel for NEON programming examples
- Linux kernel programming talks from conferences (FOSDEM, Linux Plumbers)

---

## ✅ Deliverables

By the end of Sprint 3, you should have:

1. **Working B+ tree engine** with all core operations
2. **Character device** `/dev/storage-btree` with IOCTL interface
3. **ARM NEON optimization** for key comparison and node operations
4. **Range query support** with iterator interface
5. **Comprehensive benchmarks** comparing with hash table
6. **Performance analysis** showing logarithmic scaling
7. **Memory-efficient implementation** with proper caching
8. **Complete documentation** with usage examples

---

## 🚀 Next Sprint Preview

Sprint 4 will build an LSM tree storage engine:
- Create `/dev/storage-lsm` for write-optimized storage
- Implement memtable, SSTable levels, and compaction
- Add background kernel threads for level management
- Compare write vs read optimization trade-offs

The ordered nature of your B+ tree will help understand LSM tree merge operations!

---

*Remember: B+ trees excel at range queries and ordered access. Design with cache locality and sequential access patterns in mind.*