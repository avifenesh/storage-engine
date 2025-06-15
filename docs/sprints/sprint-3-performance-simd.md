# Sprint 3: Performance Structures – Ring Buffers & SIMD

## Overview
This sprint focuses on high-performance data structures and vector operations optimized for ARM64. Implement efficient memory management with ring buffers and leverage ARM NEON SIMD for vector similarity calculations.

## Tasks

### Issue #11: Implement ring buffer with FIFO eviction
**Learning Objectives:**
- Design memory-efficient circular buffers
- Implement FIFO cache eviction strategy
- Optimize for kernel space constraints

**Ring Buffer Design:**
```c
struct ring_buffer {
    void **entries;
    size_t head;
    size_t tail;
    size_t capacity;
    size_t count;
    spinlock_t lock;
};
```

**FIFO Eviction Logic:**
- Circular index management: `(head + 1) % capacity`
- Atomic updates for thread safety
- Memory barrier considerations for ARM64
- Efficient full/empty detection

**Kernel Integration:**
- Use `kfifo` API for production implementations
- Custom implementation for learning purposes
- Integration with vector store for LRU management

**Resources:**
- [Circular buffer implementation guide](https://www.embedded.com/c-language-techniques-for-circular-buffers/)
- [Kernel kfifo documentation](https://www.kernel.org/doc/html/latest/core-api/kfifo.html)

---

### Issue #12: Study LRU cache eviction strategies
**Learning Objectives:**
- Understand cache replacement algorithms
- Compare LRU vs FIFO performance characteristics
- Design for high-frequency vector operations

**LRU Implementation Approaches:**
1. **Doubly Linked List + Hash Map:**
   ```c
   struct lru_node {
       u32 key;
       void *data;
       struct list_head lru_list;
       struct hlist_node hash_node;
   };
   ```

2. **Approximated LRU (Clock Algorithm):**
   - Single bit per entry for access tracking
   - Lower memory overhead
   - Good performance for most workloads

3. **Segmented LRU:**
   - Hot/warm/cold segments
   - Reduces lock contention
   - Better for concurrent access patterns

**Performance Considerations:**
- Cache line alignment for ARM64
- Minimize atomic operations
- Consider NUMA topology on multi-core systems

**Resources:**
- [Linux memory management algorithms](https://linux-mm.org/PageReplacementAlgorithms)
- [LRU implementation examples](https://gist.github.com/marcosfelt/7974411)

---

### Issue #13: Implement basic cosine similarity in C
**Learning Objectives:**
- Understand vector similarity mathematics
- Implement baseline scalar version
- Prepare for SIMD optimization comparison

**Mathematical Foundation:**
```
cosine_similarity(A, B) = (A · B) / (||A|| × ||B||)

Where:
- A · B = Σ(A[i] × B[i])  (dot product)
- ||A|| = √(Σ(A[i]²))     (magnitude)
```

**Scalar Implementation:**
```c
float cosine_similarity_scalar(const float *a, const float *b, size_t dim) {
    float dot_product = 0.0f;
    float norm_a = 0.0f;
    float norm_b = 0.0f;
    
    for (size_t i = 0; i < dim; i++) {
        dot_product += a[i] * b[i];
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }
    
    return dot_product / (sqrtf(norm_a) * sqrtf(norm_b));
}
```

**Optimization Opportunities:**
- Loop unrolling for better instruction pipelining
- Precomputed vector norms for database vectors
- Numerical stability considerations (avoid division by zero)

**Resources:**
- [Vector similarity algorithms](https://www.geeksforgeeks.org/program-find-similarity-two-strings/)

---

### Issue #14: Implement cosine similarity with NEON SIMD intrinsics
**Learning Objectives:**
- Master ARM NEON intrinsics programming
- Achieve 4x performance improvement with SIMD
- Understand ARM64 vector register usage

**NEON Implementation Strategy:**
```c
#include <arm_neon.h>

float cosine_similarity_neon(const float *a, const float *b, size_t dim) {
    float32x4_t sum_dot = vdupq_n_f32(0.0f);
    float32x4_t sum_a2 = vdupq_n_f32(0.0f);
    float32x4_t sum_b2 = vdupq_n_f32(0.0f);
    
    size_t simd_end = (dim / 4) * 4;
    
    // SIMD loop - process 4 elements at once
    for (size_t i = 0; i < simd_end; i += 4) {
        float32x4_t va = vld1q_f32(&a[i]);
        float32x4_t vb = vld1q_f32(&b[i]);
        
        sum_dot = vfmaq_f32(sum_dot, va, vb);  // Fused multiply-add
        sum_a2 = vfmaq_f32(sum_a2, va, va);
        sum_b2 = vfmaq_f32(sum_b2, vb, vb);
    }
    
    // Horizontal reduction
    float dot_product = vaddvq_f32(sum_dot);
    float norm_a_sq = vaddvq_f32(sum_a2);
    float norm_b_sq = vaddvq_f32(sum_b2);
    
    // Handle remaining elements (scalar tail)
    for (size_t i = simd_end; i < dim; i++) {
        dot_product += a[i] * b[i];
        norm_a_sq += a[i] * a[i];
        norm_b_sq += b[i] * b[i];
    }
    
    return dot_product / (sqrtf(norm_a_sq) * sqrtf(norm_b_sq));
}
```

**NEON Optimization Techniques:**
- **Fused Multiply-Add (FMA):** `vfmaq_f32()` for single-cycle multiply-add
- **Vector Load/Store:** `vld1q_f32()`, `vst1q_f32()` for aligned access
- **Horizontal Reduction:** `vaddvq_f32()` to sum vector elements
- **Prefetching:** Software prefetch for large datasets

**Performance Considerations:**
- Ensure 16-byte alignment for vector loads
- Minimize data movement between NEON and ARM registers
- Consider cache warming for repeated operations
- Profile with `perf` to validate improvements

**Resources:**
- [ARM NEON programmer's guide](https://developer.arm.com/documentation/den0018/a/NEON-Intrinsics/)
- [NEON intrinsics reference](https://documentation-service.arm.com/static/67a384f0e80ab6345cace1af)
- [Practical NEON examples](https://github.com/thenifty/neon-guide)

## Performance Benchmarking

### Expected Improvements:
- **Ring Buffer vs Linear Search:** 10-100x improvement for cache operations
- **NEON vs Scalar:** 3-4x improvement for vector similarity
- **Combined Optimizations:** 50-200x improvement for top-k queries

### Measurement Strategy:
```c
// Benchmark framework
struct benchmark_result {
    uint64_t cycles;
    uint64_t instructions;
    double throughput_ops_per_sec;
};

void benchmark_cosine_similarity(void) {
    // Use ARM cycle counter (CNTVCT_EL0)
    // Compare scalar vs NEON implementations
    // Measure cache miss rates
}
```

## Sprint Goals
- Implement efficient ring buffer for vector caching
- Master ARM NEON SIMD programming
- Achieve measurable performance improvements
- Build foundation for real-time vector search

## Architecture Progress
```
┌─────────────────┐    IOCTL     ┌─────────────────┐
│   Rust Client   │ ───────────▶ │  Kernel Module  │
│   (User Space)  │              │   (/dev/vss)    │
└─────────────────┘              └─────────────────┘
                                          │
                                          ▼
                                 ┌─────────────────┐
                                 │  Vector Store   │
                                 │   + Ring Buffer │ ← New!
                                 │   + NEON SIMD   │ ← New!
                                 └─────────────────┘
```

## Next Sprint Preview
Sprint 4 will focus on advanced kernel memory management, slab allocators, and building the complete integration pipeline with optional Netlink communication.
