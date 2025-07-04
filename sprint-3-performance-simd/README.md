# Sprint 3: Performance & SIMD - Practice Area

This directory is for **YOU** to implement the Sprint 3 performance optimization tasks.

## Your Tasks

### Issue #11: Ring buffer with FIFO/LRU eviction
**File to create**: `ring_buffer.c`
- **BUILD IT YOURSELF** - no copy-paste!
- Reference kernel kfifo patterns

### Issue #12: Cosine similarity (standard + NEON SIMD)
**File to create**: `cosine_similarity.c`
- First implement standard floating point version
- Then optimize with ARM NEON intrinsics

### Issue #13: SIMD-accelerated vector operations
**File to create**: `simd_ops.c` 
- Practice with float32x4_t ARM NEON types
- Implement dot product, magnitude calculations

### Issue #14: Performance testing framework
**File to create**: `perf_tests.c`
- Create benchmarks for your implementations

## Learning Resources
- [ARM NEON Intrinsics Guide](https://developer.arm.com/architectures/instruction-sets/intrinsics/)
- [Linux Kernel Ring Buffers](https://docs.kernel.org/core-api/circular-buffers.html)
- [SIMD Optimization Techniques](https://www.intel.com/content/www/us/en/developer/articles/technical/introduction-to-intel-advanced-vector-extensions.html)

## NEON Practice Tips
- Start with simple operations: addition, multiplication
- Use `vld1q_f32()` and `vst1q_f32()` for loading/storing
- Remember 128-bit alignment requirements
- Test on ARM64 hardware or emulation

**Key Point**: Performance optimization requires UNDERSTANDING the underlying concepts first!
