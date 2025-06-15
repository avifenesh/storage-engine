# Sprint 1: C Core Refresh & Memory Fundamentals

## Overview
This sprint focuses on refreshing core C programming concepts, memory management, and foundational knowledge needed for kernel development. All tasks build towards understanding memory operations essential for high-performance vector operations.

## Tasks

### Issue #1: Review pointer operations in C ✅ COMPLETED
**File:** `print_address.c`
**Learning Objectives:**
- Understand pointer arithmetic and dereferencing
- Practice memory address manipulation
- Review pointer-to-pointer concepts

**Key Concepts:**
- Pointer declarations: `int *ptr`, `void *generic_ptr`
- Address-of operator: `&variable`
- Dereference operator: `*ptr`
- Pointer arithmetic: `ptr + 1`, `ptr++`
- Function pointers: `int (*func_ptr)(int, int)`

**Resources:**
- [Pointer operations guide](https://beej.us/guide/bgc/html/split/pointers.html)

---

### Issue #2: Build dynamic array (malloc, realloc, free) ✅ COMPLETED
**File:** `dynamic_array.c`
**Learning Objectives:**
- Implement proper memory management patterns
- Understand malloc/realloc/free lifecycle
- Handle memory allocation failures gracefully

**Key Concepts:**
```c
typedef struct {
    void **data;
    size_t size;
    size_t capacity;
} dynamic_array_t;
```

**Memory Management Pattern:**
```c
void* ptr = malloc(size);
if (!ptr) {
    return -ENOMEM; // Handle allocation failure
}
// ... use ptr
free(ptr);
ptr = NULL; // Prevent use-after-free
```

**Resources:**
- [Dynamic memory tutorial](https://www.learn-c.org/en/Dynamic_memory)

---

### Issue #3: Study memory alignment and struct padding ✅ COMPLETED
**Learning Objectives:**
- Understand memory alignment requirements
- Learn about struct padding and packing
- Optimize data structures for cache performance

**Key Concepts:**
- Natural alignment: data aligned to its size boundary
- Padding: compiler-inserted bytes for alignment
- `__attribute__((packed))`: remove padding
- `__attribute__((aligned(N)))`: force specific alignment
- Cache line considerations (64 bytes on most ARM64)

**ARM64 Specifics:**
- 128-bit vector registers require 16-byte alignment
- Load/store multiple instructions benefit from alignment
- Cache line size: typically 64 bytes

**Resources:**
- [Data structure alignment reference](https://www.geeksforgeeks.org/data-structure-alignment-in-c/)

---

### Issue #4: Build aligned struct for 128-bit vectors ✅ COMPLETED
**File:** `128_aligned_vec.c`
**Learning Objectives:**
- Create ARM NEON-optimized data structures
- Understand SIMD alignment requirements
- Implement vector operations preparation

**Key Implementation:**
```c
typedef struct {
    float data[4];
} __attribute__((aligned(16))) vec4_t;
```

**ARM NEON Integration:**
- `#include <arm_neon.h>`
- `float32x4_t` for 4-way SIMD operations
- Alignment ensures efficient NEON load/store operations

**Resources:**
- Implementation in `128_aligned_vec.c`

---

### Issue #5: Learn function pointers for callback dispatchers 🔄 IN PROGRESS
**File:** `vec_dispatcher.c`
**Learning Objectives:**
- Implement callback-based function dispatch
- Create flexible operation interfaces
- Prepare for kernel-style function tables

**Key Concepts:**
```c
typedef int (*operation_func_t)(const void *a, const void *b, void *result);

typedef struct {
    const char *name;
    operation_func_t func;
} operation_dispatch_t;
```

**Use Cases:**
- Vector operation dispatch (add, multiply, dot product)
- Kernel file operations structure
- Plugin-style architecture

**Resources:**
- [Function pointer guide](https://www.geeksforgeeks.org/function-pointer-in-c/)

---

### Issue #6: Setup Makefile with GDB integration 📋 PENDING
**Learning Objectives:**
- Create ARM64-optimized build system
- Integrate debugging and profiling tools
- Support both user-space and kernel builds

**Build Flags:**
```makefile
CFLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare \
         -std=gnu11 -fno-strict-aliasing -march=armv8-a -g -O2
```

**Debug Integration:**
- GDB symbols: `-g`
- Valgrind memory checking
- Assembly output generation
- Perf profiling support

**Resources:**
- [Makefile tutorial](https://makefiletutorial.com/)
- [GDB cheat sheet](https://freecoder.dev/gdb-cheat-sheet/)
- [Valgrind guide](https://valgrind.org/docs/manual/quick-start.html)

## Sprint Goals
- Master C memory management fundamentals
- Understand ARM64/NEON alignment requirements
- Build foundation for kernel development
- Establish development and debugging workflow

## Next Sprint Preview
Sprint 2 will focus on system programming with IOCTL interfaces and character device drivers, building on the memory management foundation established here.
