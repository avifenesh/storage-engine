# Sprint 2: Hash Table Storage Engine

## Overview
Build a high-performance kernel-space hash table storage engine with ARM NEON optimization. Create `/dev/storage-hash` character device for database operations.

**Duration**: 4 weeks  
**Prerequisites**: Completed Sprint 1 (C fundamentals)

---

## 🎯 Learning Objectives

1. **Master hash table algorithms** - Implement collision resolution and dynamic resizing
2. **Build kernel character devices** - Create `/dev/storage-hash` with IOCTL interface
3. **Optimize with ARM NEON** - Use SIMD instructions for hash functions and key comparison
4. **Design storage operations** - Efficient put/get/delete with memory management
5. **Handle concurrency** - Thread-safe operations with proper locking

Tip: Start with a simple, correct userspace implementation, then port to kernel space. Add SIMD and concurrency last.

---

## 📋 Sprint Issues

### Issue #27: Implement Hash Table with Collision Resolution (Prototype ➜ Kernel)
**File**: `sprint-2-hash-storage/hash_engine.c`

**Requirements**:
- Implement either linear probing or chaining (pick one to start)
- Dynamic resizing at 75% load factor
- Atomic counters for statistics
- Proper memory management with kmalloc/kfree

**Example Interface**:
```c
struct hash_engine {
    struct hash_bucket *buckets;
    uint32_t bucket_count;
    spinlock_t engine_lock;
    atomic_t item_count;
    atomic_t total_memory;
};

struct hash_entry {
    void *key;
    size_t key_len;
    void *value;
    size_t value_len;
    struct hash_entry *next;
};

// Core operations (userspace first, then gate kernel-only code under __KERNEL__)
int hash_put(const void *key, size_t key_len, const void *value, size_t value_len);
int hash_get(const void *key, size_t key_len, void **value, size_t *value_len);
int hash_delete(const void *key, size_t key_len);
int hash_get_stats(uint32_t *item_count, uint32_t *bucket_count, uint32_t *memory_usage);
```

**Success Criteria**:
- Handle 10,000 concurrent operations without corruption
- O(1) average lookup time
- Memory usage <150% of raw data size
- Collision rate <10% with good hash function

Lab Steps:
- Step 1: Implement a userspace hash table with linear probing (no locks). Add simple tests in `sprint-2-hash-storage/tests/hash_tests.c`.
- Step 2: Add resizing and verify load factor thresholds by printing stats every 1,000 inserts.
- Step 3: Add pthread-based concurrent put/get tests; observe race conditions; then add fine-grained bucket locks or a global lock.
- Step 4: Port to kernel by replacing malloc/free with kmalloc/kfree and using spinlocks/atomics. Keep the userspace version intact for benchmarking.

---

### Issue #28: Build Character Device with IOCTL Interface
**File**: `sprint-2-hash-storage/hash_device.c`

**Requirements**:
- Character device `/dev/storage-hash`
- IOCTL commands for PUT/GET/DELETE/STATS
- Safe user-kernel data transfer
- Proper error handling and validation

**IOCTL Interface**:
```c
#define HASH_IOC_MAGIC 'H'
#define HASH_IOC_PUT    _IOW(HASH_IOC_MAGIC, 1, struct hash_operation)
#define HASH_IOC_GET    _IOWR(HASH_IOC_MAGIC, 2, struct hash_operation)
#define HASH_IOC_DELETE _IOW(HASH_IOC_MAGIC, 3, struct hash_operation)
#define HASH_IOC_STATS  _IOR(HASH_IOC_MAGIC, 4, struct hash_stats)

struct hash_operation {
    void __user *key;
    size_t key_len;
    void __user *value;
    size_t value_len;
};

struct hash_stats {
    uint32_t item_count;
    uint32_t bucket_count;
    uint32_t memory_usage;
};
```

**Success Criteria**:
- All IOCTL operations work correctly
- No kernel panics with malformed input  
- Proper user space pointer validation
- Clean module load/unload

Checkpoint Demo:
- Write a tiny userspace program that opens `/dev/storage-hash` and performs PUT/GET/DELETE using your IOCTLs. Print results and error codes.

---

### Issue #29: ARM NEON Hash Function Optimization
**File**: `sprint-2-hash-storage/neon_hash.c`

**Requirements**:
- SIMD-optimized hash functions
- Vectorized key comparison
- Batch processing capabilities
- Fallback for non-ARM64 systems

**NEON Hash Implementation**:
```c
// SIMD hash function processing 16 bytes at once
uint32_t neon_hash_function(const void *key, size_t len);

// Batch hash multiple keys simultaneously  
void neon_hash_batch(const void **keys, size_t *key_lens, 
                     uint32_t *hashes, size_t count);

// SIMD key comparison
int neon_key_compare(const void *key1, size_t key1_len, 
                     const void *key2, size_t key2_len);

// Hash quality measurement
uint32_t neon_hash_distribution_test(const void **keys, size_t *key_lens,
                                     size_t key_count, uint32_t bucket_count);
```

**Success Criteria**:
- 3x speedup over scalar hash functions
- Good distribution with low collision rate
- Handles variable-length keys correctly
- Performance scales with ARM NEON capabilities

Study Targets:
- Inspect compiler-generated assembly with `make asm-learn` and compare scalar vs NEON versions.
- Align hot buffers to 16 bytes; measure impact.

---

### Issue #30: Create Comprehensive Test Suite
**File**: `sprint-2-hash-storage/tests/hash_tests.c`

**Requirements**:
- Unit tests for all hash operations
- Performance benchmarks
- Stress testing with concurrent access
- Memory leak detection

**Test Categories**:
```c
// Basic operations
int test_basic_operations(void);

// Error conditions  
int test_error_conditions(void);

// Performance measurement
int test_performance(void);

// Concurrent access
int test_concurrent_operations(void);
```

**Performance Targets (Raspberry Pi 4/5)**:
- Hash reads: >10,000 operations/second
- Hash writes: >3,000 operations/second
- Memory usage: <150% of raw data size
- Collision rate: <10% with uniform keys

---

## 🛠️ Implementation Guide

### Week 1: Core Hash Engine
1. Implement basic hash table structure
2. Add collision resolution with chaining
3. Create put/get/delete operations
4. Add dynamic resizing logic
5. Test with single-threaded access

### Week 2: Character Device Interface
1. Create kernel module skeleton
2. Implement character device registration
3. Add IOCTL command handlers
4. Implement user-kernel data transfer
5. Test with simple user program

### Week 3: ARM NEON Optimization
1. Study ARM NEON intrinsics
2. Implement SIMD hash functions
3. Add vectorized key comparison
4. Create batch processing functions
5. Benchmark against scalar implementation

### Week 4: Testing and Integration
1. Build comprehensive test suite
2. Add performance benchmarks
3. Test concurrent access patterns
4. Fix bugs and optimize performance
5. Document API and usage

Learning Checklist:
- Can explain trade-offs of linear probing vs chaining
- Can show atomics and locking used correctly in kernel code
- Can demonstrate measurable speedups from NEON

---

## 🔒 What’s Provided vs What You Build

- Provided (scaffolding only):
  - Minimal userspace stubs in `sprint-2-hash-storage/` (no working hash logic)
  - Placeholder test harness file in `tests/` with TODOs
  - Make targets to build/run your tests
- You Build (for learning):
  - Hash table algorithm (probing/chaining), resizing, and stats
  - Correctness under concurrency (locking/atomics)
  - Kernel character device and IOCTL handlers
  - NEON optimizations and measurable benchmarks

No solutions are provided; only QA scaffolding to validate your own implementation.

---

## 🚀 Expert Track (Optional, Fast Path)

- Probing: Robin Hood hashing with tombstones; bounded probe length histograms.
- Resizing: incremental, lock‑striped or RCU‑based without stop‑the‑world.
- Concurrency: per‑bucket spinlocks or lock striping; try read‑mostly lock‑free gets.
- Hashing: feature‑gated CRC32/NEON with runtime dispatch; alignment + tail handling.
- QA: property tests vs an oracle map; contention profiling; p50/p99 under 8 threads.

---

## 📏 Quality Gates & Targets

Use these as measurable finish lines for this sprint. Adjust for your hardware, but keep the ratios.

- Correctness
  - Property tests: 100k random ops (mix of put/get/delete) match oracle map
  - Concurrency: 8 threads, 1M total ops without corruption or deadlocks
  - Load factor stays within [0.2, 0.75] with automatic resize
- Hash quality
  - Uniform random keys: bucket occupancy stddev < 15% of mean
  - Probe length (open addressing): p99 ≤ 8, max ≤ 32
  - Collision rate (chaining): average chain length ≤ 1.5 at 0.75 load factor
- Performance (Raspberry Pi 4/5 baseline; scale relatively on other HW)
  - Reads: ≥ 10k ops/s single-thread; ≥ 50k ops/s at 8 threads
  - Writes: ≥ 3k ops/s single-thread; ≥ 15k ops/s at 8 threads
  - NEON/CRC32 path: ≥ 2.5x speedup vs scalar hashing on ≥16B keys
- Memory
  - Overhead ≤ 150% of raw KV footprint at 0.6–0.75 load factor

---

## 🧪 Measurement Checklist

- Build/run expert harness: `make test-expert` (runs `build/hash_prop`)
- Throughput: add a simple timed loop in your hash tests; record read/write ops/s (1 thread, then 8 threads)
- Hash quality: export bucket occupancy/probe length stats; fill `metrics.json` fields under `sprint2_hash`
- Record environment: CPU model, core count, kernel version in a comment near the metrics

---

## 📊 Testing Strategy

### Unit Tests
```c
// test basic hash operations
void test_basic_hash_ops() {
    // Test PUT operation
    int result = hash_put("key1", 4, "value1", 6);
    assert(result == 0);
    
    // Test GET operation
    void *value;
    size_t value_len;
    result = hash_get("key1", 4, &value, &value_len);
    assert(result == 0);
    assert(value_len == 6);
    assert(memcmp(value, "value1", 6) == 0);
    
    // Test DELETE operation
    result = hash_delete("key1", 4);
    assert(result == 0);
    
    // Verify deletion
    result = hash_get("key1", 4, &value, &value_len);
    assert(result == -ENOENT);
}
```

### Performance Tests
```bash
# Load kernel module
sudo make load

# Run performance benchmarks
make test

# Check statistics
dmesg | grep "hash_engine"

# Unload module
sudo make unload
```

### SIMD Verification
```c
// Compare SIMD vs scalar results
void verify_simd_correctness() {
    const char *test_keys[] = {"test1", "test2", "test3"};
    size_t key_lens[] = {5, 5, 5};
    uint32_t scalar_hashes[3], neon_hashes[3];
    
    // Compute with both methods
    for (int i = 0; i < 3; i++) {
        scalar_hashes[i] = scalar_hash_function(test_keys[i], key_lens[i]);
    }
    neon_hash_batch((const void**)test_keys, key_lens, neon_hashes, 3);
    
    // Results should match
    for (int i = 0; i < 3; i++) {
        assert(scalar_hashes[i] == neon_hashes[i]);
    }
}
```

---

## 🐛 Common Issues and Solutions

### Issue: Hash table performance degrades
**Solution**: Monitor load factor, implement dynamic resizing, tune hash function

### Issue: IOCTL operations fail
**Solution**: Check user pointer validation, verify IOCTL command numbers, test permissions

### Issue: ARM NEON code crashes
**Solution**: Check kernel_neon_begin/end pairs, verify data alignment, handle edge cases

### Issue: Memory leaks in kernel
**Solution**: Use kmemleak, match kmalloc/kfree calls, implement proper cleanup

---

## 📚 Resources

### Hash Table Theory
- [Hash Tables Explained](https://benhoyt.com/writings/hash-table-in-c/)
- [Collision Resolution Strategies](https://en.wikipedia.org/wiki/Hash_table#Collision_resolution)
- [Performance Analysis](https://github.com/attractivechaos/klib)

### Kernel Programming
- [Linux Device Drivers](https://lwn.net/Kernel/LDD3/) - Character devices
- [Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)
- [IOCTL Tutorial](https://embetronicx.com/tutorials/linux/device-drivers/ioctl-tutorial-in-linux/)

### ARM NEON Optimization
- [ARM NEON Programmer's Guide](https://developer.arm.com/documentation/102474/latest/)
- [NEON Intrinsics Reference](https://arm-software.github.io/acle/neon_intrinsics/advsimd.html)
- [SIMD Hash Functions](https://lemire.me/blog/2021/01/29/arm-neon-programming-quick-reference/)

---

## ✅ Deliverables

By the end of Sprint 2, you should have:

1. **Working hash engine** with collision resolution
2. **Character device** `/dev/storage-hash` with IOCTL interface
3. **ARM NEON optimization** for hash operations
4. **Comprehensive tests** with performance benchmarks
5. **Memory-safe implementation** with no leaks
6. **Documentation** with API reference and examples
7. **Performance targets met** on Raspberry Pi hardware
8. **Clean code** following Linux kernel style

---

## 🚀 Next Sprint Preview

Sprint 3 will build a B+ tree storage engine:
- Create `/dev/storage-btree` for sorted data storage
- Implement B+ tree algorithms in kernel space
- Add range query support and tree traversal
- Optimize node operations with ARM NEON

Your hash table foundation will inform the design patterns for more complex storage engines!

---

*Remember: Hash tables are the foundation of many storage systems. Master them well before moving to more complex data structures.*
