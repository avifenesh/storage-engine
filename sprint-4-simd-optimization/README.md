# Sprint 4: SIMD Text Optimization

## Overview
This sprint optimizes text processing operations using ARM NEON SIMD instructions for significant performance improvements.

## Goals
- Implement SIMD string operations
- Optimize hash computations with NEON
- Create comprehensive benchmarks
- Achieve 3-4x speedup on key operations

## Key Files
- `neon_string_ops.c` - SIMD string comparison and processing
- `neon_hash.c` - Vectorized hash functions
- `benchmarks.c` - Performance comparison framework
- `test_simd.c` - Correctness verification

## Getting Started
1. Read the sprint documentation: `../docs/sprints/sprint-4-simd-optimization.md`
2. Learn ARM NEON intrinsics basics
3. Start with Issue #35 (string operations)
4. Always verify correctness against scalar code

## Building
```bash
# Compile with NEON support
make sprint4 CFLAGS="-march=armv8-a+simd"

# Run correctness tests
make test-simd

# Run benchmarks
make bench-simd
```

## ARM NEON Quick Reference
```c
// Load/Store
uint8x16_t vld1q_u8(const uint8_t *ptr);  // Load 16 bytes
void vst1q_u8(uint8_t *ptr, uint8x16_t val);  // Store 16 bytes

// Comparison
uint8x16_t vceqq_u8(uint8x16_t a, uint8x16_t b);  // Compare equal
uint8x16_t vcgeq_u8(uint8x16_t a, uint8x16_t b);  // Compare >=

// Arithmetic
uint8x16_t vaddq_u8(uint8x16_t a, uint8x16_t b);  // Add
uint32x4_t vmulq_u32(uint32x4_t a, uint32x4_t b);  // Multiply

// Logical
uint8x16_t vandq_u8(uint8x16_t a, uint8x16_t b);  // AND
uint8x16_t vorrq_u8(uint8x16_t a, uint8x16_t b);  // OR
```

## Performance Targets
- String comparison: 4x speedup
- Tokenization: 3x speedup
- Hash computation: 3x speedup
- Maintain correctness with all edge cases

## Optimization Tips
1. Process at least 16 bytes per SIMD operation
2. Handle alignment carefully
3. Use scalar code for small data (<32 bytes)
4. Prefetch data for better cache usage
5. Minimize lane extraction operations

## Benchmarking
```bash
# Compare scalar vs SIMD
./benchmarks --compare

# Generate performance report
./benchmarks --report > performance_report.txt

# Profile with perf
perf record -g ./benchmarks
perf report
```

## Common Pitfalls
- Forgetting to handle unaligned data
- Not checking for buffer overruns
- Assuming little-endian byte order
- Ignoring edge cases (empty strings, single character)

## Resources
- [ARM NEON Programmer's Guide](https://developer.arm.com/documentation/102474/latest/)
- [NEON Intrinsics Reference](https://developer.arm.com/architectures/instruction-sets/intrinsics/)
- [Practical NEON Examples](https://www.codeproject.com/Articles/5294663/Neon-Intrinsics-for-Optimized-Math-Networking-and)