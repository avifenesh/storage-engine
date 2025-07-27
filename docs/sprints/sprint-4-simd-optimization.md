# Sprint 4: SIMD Text Optimization

## Overview
Optimize text processing operations using ARM NEON SIMD instructions. Learn to identify performance bottlenecks and apply vectorization techniques for significant speedups.

**Duration**: 2-3 weeks  
**Prerequisites**: Completed Sprint 3 (kernel character device)

---

## 🎯 Learning Objectives

1. **Master ARM NEON intrinsics** - Use SIMD instructions for text processing
2. **Optimize string operations** - Vectorize comparison, search, and hashing
3. **Profile and measure** - Identify hot paths and quantify improvements
4. **Compare implementations** - Benchmark scalar vs SIMD vs kernel
5. **Understand CPU architecture** - Cache effects and instruction pipelining

---

## 📋 Sprint Issues

### Issue #35: ARM NEON String Comparison Optimization
**File**: `sprint-4-simd-optimization/neon_string_ops.c`

**Requirements**:
- Implement SIMD string comparison functions
- Optimize character classification (alpha, numeric, whitespace)
- Vectorize case conversion (upper/lower)
- Handle string alignment and tail processing

**Core NEON String Functions**:
```c
#include <arm_neon.h>
#include <string.h>

// SIMD string comparison
int neon_strcmp(const char *s1, const char *s2) {
    // Handle unaligned start
    while (((uintptr_t)s1 & 15) && *s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    
    // Main SIMD loop - process 16 bytes at a time
    if (*s1 && *s2) {
        const uint8x16_t *p1 = (const uint8x16_t *)s1;
        const uint8x16_t *p2 = (const uint8x16_t *)s2;
        
        while (1) {
            uint8x16_t v1 = vld1q_u8((const uint8_t *)p1);
            uint8x16_t v2 = vld1q_u8((const uint8_t *)p2);
            
            // Check for null bytes
            uint8x16_t zero = vdupq_n_u8(0);
            uint8x16_t has_zero = vceqq_u8(v1, zero);
            
            // Compare strings
            uint8x16_t diff = vceqq_u8(v1, v2);
            
            // Check if all bytes match and no nulls
            if (vmaxvq_u8(has_zero) || !vminvq_u8(diff)) {
                // Fall back to scalar for final comparison
                return strcmp((const char *)p1, (const char *)p2);
            }
            
            p1++; p2++;
        }
    }
    
    return *s1 - *s2;
}

// SIMD lowercase conversion
void neon_to_lower(char *str, size_t len) {
    const uint8x16_t A = vdupq_n_u8('A');
    const uint8x16_t Z = vdupq_n_u8('Z');
    const uint8x16_t diff = vdupq_n_u8('a' - 'A');
    
    size_t i = 0;
    
    // Process 16 characters at a time
    for (; i + 16 <= len; i += 16) {
        uint8x16_t chars = vld1q_u8((uint8_t *)&str[i]);
        
        // Check if chars are uppercase (A <= char <= Z)
        uint8x16_t ge_A = vcgeq_u8(chars, A);
        uint8x16_t le_Z = vcleq_u8(chars, Z);
        uint8x16_t is_upper = vandq_u8(ge_A, le_Z);
        
        // Convert uppercase to lowercase
        uint8x16_t to_add = vandq_u8(is_upper, diff);
        chars = vaddq_u8(chars, to_add);
        
        vst1q_u8((uint8_t *)&str[i], chars);
    }
    
    // Handle remaining characters
    for (; i < len; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 'a' - 'A';
        }
    }
}

// SIMD whitespace detection
size_t neon_find_whitespace(const char *str, size_t len) {
    const uint8x16_t space = vdupq_n_u8(' ');
    const uint8x16_t tab = vdupq_n_u8('\t');
    const uint8x16_t newline = vdupq_n_u8('\n');
    const uint8x16_t cr = vdupq_n_u8('\r');
    
    size_t i = 0;
    
    for (; i + 16 <= len; i += 16) {
        uint8x16_t chars = vld1q_u8((uint8_t *)&str[i]);
        
        // Check for any whitespace
        uint8x16_t is_space = vceqq_u8(chars, space);
        uint8x16_t is_tab = vceqq_u8(chars, tab);
        uint8x16_t is_newline = vceqq_u8(chars, newline);
        uint8x16_t is_cr = vceqq_u8(chars, cr);
        
        // Combine all whitespace checks
        uint8x16_t is_whitespace = vorrq_u8(is_space, is_tab);
        is_whitespace = vorrq_u8(is_whitespace, is_newline);
        is_whitespace = vorrq_u8(is_whitespace, is_cr);
        
        // Check if any whitespace found
        if (vmaxvq_u8(is_whitespace)) {
            // Find exact position with scalar code
            for (size_t j = 0; j < 16; j++) {
                char c = str[i + j];
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                    return i + j;
                }
            }
        }
    }
    
    // Check remaining bytes
    for (; i < len; i++) {
        char c = str[i];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            return i;
        }
    }
    
    return len;  // No whitespace found
}
```

**Tokenization with NEON**:
```c
// SIMD-accelerated tokenizer
typedef struct {
    const char *start;
    size_t length;
} token_t;

size_t neon_tokenize(const char *text, size_t len, token_t *tokens, 
                    size_t max_tokens) {
    size_t token_count = 0;
    size_t i = 0;
    
    while (i < len && token_count < max_tokens) {
        // Skip whitespace using SIMD
        size_t ws_pos = neon_find_non_whitespace(&text[i], len - i);
        i += ws_pos;
        
        if (i >= len) break;
        
        // Find next whitespace (end of token)
        size_t token_start = i;
        size_t token_end = i + neon_find_whitespace(&text[i], len - i);
        
        // Store token
        tokens[token_count].start = &text[token_start];
        tokens[token_count].length = token_end - token_start;
        token_count++;
        
        i = token_end;
    }
    
    return token_count;
}
```

**Success Criteria**:
- 4x speedup for string operations on aligned data
- Correct results matching scalar implementation
- Handles unaligned data and edge cases
- Measurable improvement in tokenization

---

### Issue #36: SIMD Hash Computation for Term Processing
**File**: `sprint-4-simd-optimization/neon_hash.c`

**Requirements**:
- Implement vectorized hash functions
- Optimize for hash table lookups
- Reduce collision rates
- Compare with scalar implementation

**NEON Hash Implementation**:
```c
// SIMD FNV-1a hash
uint32_t neon_fnv1a_hash(const char *str, size_t len) {
    const uint32_t FNV_PRIME = 0x01000193;
    const uint32_t FNV_OFFSET = 0x811c9dc5;
    
    // Initialize 4 hash values for parallel processing
    uint32x4_t hash = vdupq_n_u32(FNV_OFFSET);
    uint32x4_t prime = vdupq_n_u32(FNV_PRIME);
    
    size_t i = 0;
    
    // Process 16 bytes at a time (4 bytes per hash lane)
    for (; i + 16 <= len; i += 16) {
        uint8x16_t chars = vld1q_u8((uint8_t *)&str[i]);
        
        // Split into 4 32-bit lanes
        uint32x4_t c1 = vmovl_u16(vget_low_u16(vmovl_u8(vget_low_u8(chars))));
        uint32x4_t c2 = vmovl_u16(vget_high_u16(vmovl_u8(vget_low_u8(chars))));
        uint32x4_t c3 = vmovl_u16(vget_low_u16(vmovl_u8(vget_high_u8(chars))));
        uint32x4_t c4 = vmovl_u16(vget_high_u16(vmovl_u8(vget_high_u8(chars))));
        
        // XOR and multiply
        hash = veorq_u32(hash, c1);
        hash = vmulq_u32(hash, prime);
        hash = veorq_u32(hash, c2);
        hash = vmulq_u32(hash, prime);
        hash = veorq_u32(hash, c3);
        hash = vmulq_u32(hash, prime);
        hash = veorq_u32(hash, c4);
        hash = vmulq_u32(hash, prime);
    }
    
    // Combine the 4 hash values
    uint32_t final_hash = vgetq_lane_u32(hash, 0);
    final_hash ^= vgetq_lane_u32(hash, 1);
    final_hash ^= vgetq_lane_u32(hash, 2);
    final_hash ^= vgetq_lane_u32(hash, 3);
    
    // Process remaining bytes
    for (; i < len; i++) {
        final_hash ^= (uint32_t)str[i];
        final_hash *= FNV_PRIME;
    }
    
    return final_hash;
}

// SIMD CRC32 using ARM crypto extensions
uint32_t neon_crc32_hash(const char *str, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    size_t i = 0;
    
    #ifdef __ARM_FEATURE_CRC32
    // Process 8 bytes at a time using CRC32 instructions
    for (; i + 8 <= len; i += 8) {
        uint64_t data = *(uint64_t *)&str[i];
        crc = __crc32d(crc, data);
    }
    
    // Process remaining bytes
    for (; i < len; i++) {
        crc = __crc32b(crc, str[i]);
    }
    #else
    // Fallback to software implementation
    return software_crc32(str, len);
    #endif
    
    return ~crc;
}

// Vectorized hash table lookup
int neon_hash_lookup(const char *key, size_t key_len,
                    struct hash_entry *table, size_t table_size) {
    uint32_t hash = neon_fnv1a_hash(key, key_len);
    size_t index = hash & (table_size - 1);  // Assume power of 2
    
    // Linear probing with SIMD string comparison
    while (table[index].key != NULL) {
        if (table[index].key_len == key_len &&
            neon_memcmp(table[index].key, key, key_len) == 0) {
            return index;  // Found
        }
        index = (index + 1) & (table_size - 1);
    }
    
    return -1;  // Not found
}
```

**Hash Quality Testing**:
```c
// Test hash distribution
void test_hash_distribution(hash_func_t hash_func, const char *name) {
    const size_t BUCKETS = 1024;
    size_t distribution[BUCKETS] = {0};
    
    // Hash test strings
    for (int i = 0; i < 100000; i++) {
        char str[32];
        snprintf(str, sizeof(str), "test_string_%d", i);
        uint32_t hash = hash_func(str, strlen(str));
        distribution[hash % BUCKETS]++;
    }
    
    // Calculate statistics
    double mean = 100000.0 / BUCKETS;
    double variance = 0;
    size_t min = SIZE_MAX, max = 0;
    
    for (size_t i = 0; i < BUCKETS; i++) {
        variance += pow(distribution[i] - mean, 2);
        if (distribution[i] < min) min = distribution[i];
        if (distribution[i] > max) max = distribution[i];
    }
    variance /= BUCKETS;
    
    printf("%s distribution: mean=%.2f, stddev=%.2f, min=%zu, max=%zu\n",
           name, mean, sqrt(variance), min, max);
}
```

**Success Criteria**:
- 3x speedup for hash computation
- Similar or better collision rates
- Works with variable-length strings
- Maintains good distribution properties

---

### Issue #37: Performance Benchmarking - User-space vs Kernel
**File**: `sprint-4-simd-optimization/benchmarks.c`

**Requirements**:
- Create comprehensive benchmark suite
- Compare scalar vs SIMD implementations
- Measure kernel vs userspace overhead
- Generate performance reports

**Benchmark Framework**:
```c
#include <time.h>
#include <stdio.h>
#include <arm_neon.h>

// High-resolution timer
typedef struct {
    struct timespec start;
    struct timespec end;
    const char *name;
} benchmark_t;

static inline void bench_start(benchmark_t *bench) {
    clock_gettime(CLOCK_MONOTONIC, &bench->start);
}

static inline double bench_end(benchmark_t *bench) {
    clock_gettime(CLOCK_MONOTONIC, &bench->end);
    double elapsed = (bench->end.tv_sec - bench->start.tv_sec) * 1e9;
    elapsed += (bench->end.tv_nsec - bench->start.tv_nsec);
    return elapsed / 1e6;  // Return milliseconds
}

// CPU cycle counter for fine-grained measurements
static inline uint64_t read_cpu_cycles(void) {
    uint64_t cycles;
    asm volatile("mrs %0, cntvct_el0" : "=r" (cycles));
    return cycles;
}

// Benchmark suite
typedef struct {
    void (*scalar_func)(void *);
    void (*neon_func)(void *);
    const char *name;
    void *test_data;
    size_t iterations;
} benchmark_test_t;

void run_benchmarks(benchmark_test_t *tests, size_t num_tests) {
    printf("%-30s %15s %15s %10s\n", 
           "Test", "Scalar (ms)", "NEON (ms)", "Speedup");
    printf("%s\n", 
           "------------------------------------------------------------");
    
    for (size_t i = 0; i < num_tests; i++) {
        benchmark_test_t *test = &tests[i];
        benchmark_t bench = {.name = test->name};
        
        // Warm up caches
        test->scalar_func(test->test_data);
        test->neon_func(test->test_data);
        
        // Benchmark scalar
        bench_start(&bench);
        for (size_t j = 0; j < test->iterations; j++) {
            test->scalar_func(test->test_data);
        }
        double scalar_time = bench_end(&bench);
        
        // Benchmark NEON
        bench_start(&bench);
        for (size_t j = 0; j < test->iterations; j++) {
            test->neon_func(test->test_data);
        }
        double neon_time = bench_end(&bench);
        
        // Calculate speedup
        double speedup = scalar_time / neon_time;
        
        printf("%-30s %15.3f %15.3f %10.2fx\n",
               test->name, scalar_time, neon_time, speedup);
    }
}
```

**Specific Benchmarks**:
```c
// String comparison benchmark
void bench_strcmp_scalar(void *data) {
    char **strings = (char **)data;
    strcmp(strings[0], strings[1]);
}

void bench_strcmp_neon(void *data) {
    char **strings = (char **)data;
    neon_strcmp(strings[0], strings[1]);
}

// Tokenization benchmark
void bench_tokenize_scalar(void *data) {
    tokenize_data_t *td = (tokenize_data_t *)data;
    scalar_tokenize(td->text, td->len, td->tokens, td->max_tokens);
}

void bench_tokenize_neon(void *data) {
    tokenize_data_t *td = (tokenize_data_t *)data;
    neon_tokenize(td->text, td->len, td->tokens, td->max_tokens);
}

// Kernel vs userspace comparison
void bench_kernel_vs_userspace(void) {
    int fd = open("/dev/textsearch", O_RDWR);
    if (fd < 0) {
        perror("open /dev/textsearch");
        return;
    }
    
    // Prepare test data
    const char *doc = "Sample document for kernel benchmark testing";
    struct textsearch_add_doc add_doc = {
        .doc_id = 1,
        .content = (char *)doc,
        .length = strlen(doc)
    };
    
    // Benchmark kernel IOCTL
    benchmark_t bench = {.name = "Kernel IOCTL"};
    bench_start(&bench);
    
    for (int i = 0; i < 1000; i++) {
        add_doc.doc_id = i;
        ioctl(fd, TEXTSEARCH_ADD_DOC, &add_doc);
    }
    
    double kernel_time = bench_end(&bench);
    
    // Benchmark userspace
    inverted_index_t *index = create_index(10000);
    bench_start(&bench);
    
    for (int i = 0; i < 1000; i++) {
        token_list_t *tokens = tokenize_text(doc, strlen(doc));
        add_document(index, i, tokens);
        free_token_list(tokens);
    }
    
    double userspace_time = bench_end(&bench);
    
    printf("\nKernel vs Userspace:\n");
    printf("Kernel:    %.3f ms\n", kernel_time);
    printf("Userspace: %.3f ms\n", userspace_time);
    printf("Overhead:  %.1f%%\n", 
           ((kernel_time - userspace_time) / userspace_time) * 100);
    
    free_index(index);
    close(fd);
}
```

**Performance Report Generation**:
```c
void generate_performance_report(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return;
    
    fprintf(fp, "# Text Search Engine Performance Report\n\n");
    fprintf(fp, "## System Information\n");
    fprintf(fp, "- Architecture: ARM64\n");
    fprintf(fp, "- CPU: %s\n", get_cpu_info());
    fprintf(fp, "- NEON: %s\n", has_neon() ? "Available" : "Not available");
    fprintf(fp, "- Cache: L1=%zuKB, L2=%zuKB\n\n", get_l1_size(), get_l2_size());
    
    fprintf(fp, "## Benchmark Results\n\n");
    
    // Run all benchmarks and output to file
    redirect_stdout_to_file(fp);
    run_all_benchmarks();
    restore_stdout();
    
    fprintf(fp, "\n## Optimization Recommendations\n");
    fprintf(fp, "1. Use NEON for string operations (4x speedup)\n");
    fprintf(fp, "2. Align data to 16-byte boundaries\n");
    fprintf(fp, "3. Process in batches to maximize cache usage\n");
    fprintf(fp, "4. Consider kernel overhead for small operations\n");
    
    fclose(fp);
}
```

**Success Criteria**:
- Clear performance comparisons
- Reproducible results
- Identifies optimization opportunities
- Professional report format

---

## 🛠️ Implementation Guide

### Week 1: NEON String Operations
1. Learn ARM NEON intrinsics basics
2. Implement string comparison
3. Add case conversion functions
4. Optimize tokenization
5. Verify correctness

### Week 2: Hash Optimization
1. Study hash algorithms
2. Implement SIMD hash functions
3. Test distribution quality
4. Integrate with hash tables
5. Measure lookup performance

### Week 3: Benchmarking
1. Create benchmark framework
2. Implement specific tests
3. Compare implementations
4. Generate reports
5. Document findings

---

## 📊 Testing Strategy

### Correctness Testing
```bash
# Run unit tests
make test-simd

# Verify against scalar implementation
./verify_simd --iterations=10000

# Test edge cases
./test_alignment
./test_overflow
```

### Performance Testing
```bash
# Run benchmarks
./benchmark --all --output=report.txt

# Profile with perf
perf record -g ./benchmark
perf report

# Check assembly output
objdump -d neon_string_ops.o | grep -A20 neon_strcmp
```

---

## 🐛 Common Issues and Solutions

### Issue: Incorrect results with unaligned data
**Solution**: Handle alignment explicitly, process unaligned bytes with scalar code

### Issue: No speedup observed
**Solution**: Check data alignment, ensure sufficient work per SIMD operation

### Issue: Slower than scalar for small strings
**Solution**: Add threshold check, use scalar for strings < 32 bytes

---

## 📚 Resources

### ARM NEON Documentation
- [ARM NEON Programmer's Guide](https://developer.arm.com/documentation/102474/latest/)
- [NEON Intrinsics Reference](https://developer.arm.com/architectures/instruction-sets/intrinsics/)
- [Optimization Guide](https://developer.arm.com/documentation/102458/latest/)

### Performance Analysis
- [ARM Performance Libraries](https://developer.arm.com/tools-and-software/server-and-hpc/arm-architecture-tools/arm-performance-libraries)
- [Linux perf on ARM](https://www.kernel.org/doc/html/latest/arm64/perf.html)

---

## ✅ Deliverables

By the end of Sprint 4, you should have:

1. **NEON string operations** with 4x speedup
2. **Vectorized hash functions** with good distribution
3. **Comprehensive benchmarks** comparing all implementations
4. **Performance report** with analysis and recommendations
5. **Integration** with existing codebase
6. **Documentation** of optimization techniques

---

## 🚀 Next Sprint Preview

Sprint 5 will add production features:
- Rust client library with safe API
- Advanced search features (phrases, boolean)
- Comprehensive testing framework
- Deployment documentation

Make sure optimizations are stable before adding new features!

---

*Remember: Measure first, optimize second. Not all code benefits from SIMD!*