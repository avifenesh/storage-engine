---
applyTo: '**'
---
Coding standards, domain knowledge, and preferences that AI should follow.---
applyTo: '**'
---
# C-Refresher Repository - Copilot Instructions

## Project Context & Goals

This repository is a C knowledge refresher project focused on systems programming, kernel development, and Raspberry Pi optimization. The main objective is to build an in-kernel vector store with user-space Rust clients for high-performance vector similarity search.

### Core Architecture:
- **Kernel Module (C)**: In-kernel vector database with IOCTL interface, optimized with ARM NEON SIMD
- **User-space Client (Rust)**: Communicates with kernel via IOCTL commands (`/dev/vss`)
- **Memory Store**: In-memory database containing actual objects that vectors represent
- **Platform**: ARM64/Raspberry Pi with kernel programming focus

## Development Standards

### C Programming Guidelines:
- Use kernel-style C (GNU11 standard) with strict compilation flags: `-Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare -std=gnu11 -fno-strict-aliasing -march=armv8-a`
- Follow Linux kernel coding style for all C code
- Implement proper memory management with explicit malloc/free tracking
- Use ARM NEON SIMD intrinsics for vector operations when optimizing
- Always include proper error handling and bounds checking
- Use `void *` for generic pointer casting and include proper type safety
- When a function doesn't take any params use explicit `void`
- Consider any possible optimization, even when it is about one statment that can be done more efficiently

### Kernel Programming:
- Use miscdevice interface for character device creation
- Implement IOCTL commands: `ADD_VECTOR`, `QUERY_TOP_K`, `GET_STATS`
- Use kernel memory allocators (kmalloc, potentially slab for optimization)
- Implement ring buffers with FIFO/LRU eviction strategies
- Follow Linux Device Driver (LDD3) patterns and best practices
- Use kernel-appropriate data structures (kfifo, linked lists)

### Rust Integration:
- Use the `nix` crate for IOCTL system calls
- Implement proper error handling with Result types
- Create safe wrappers around unsafe IOCTL operations
- Use serde for data serialization when communicating with external stores

### Build System:
- Use the provided Makefile with ARM64 kernel-style compilation flags
- Generate assembly files for learning and optimization analysis
- Include GDB debugging symbols and Valgrind memory checking
- Support both user-space and kernel module compilation

## Current Sprint Progress

### Sprint 1: C Core Refresh & Memory Fundamentals
- ✅ Review pointer operations in C (Issue #1 - COMPLETED)
- ✅ Build dynamic array (malloc, realloc, free) (Issue #2 - COMPLETED)
- ✅ Study memory alignment and struct padding (Issue #3 - COMPLETED)
- ✅ Build aligned struct for 128-bit vectors (Issue #4 - COMPLETED)
- 🔄 Learn function pointers for callback dispatchers (Issue #5 - IN PROGRESS)
- 📋 Setup Makefile with GDB integration (Issue #6 - PENDING)

### Sprint 2: System Programming – IOCTL & Char Devices  
- Build basic char device with miscdevice interface (Issue #7)
- Implement IOCTL protocol (ADD_VECTOR, QUERY_TOP_K) (Issue #8)
- Build Rust user-space client for /dev/vss communication (Issue #9)
- Build simple in-kernel vector store (Issue #10)

### Sprint 3: Performance Structures – Ring Buffers & SIMD
- Implement ring buffer with FIFO eviction (Issue #11)
- Study LRU cache eviction strategies (Issue #12)  
- Implement basic cosine similarity in C (Issue #13)
- Implement cosine similarity with NEON SIMD intrinsics (Issue #14)

### Sprint 4: Kernel Memory Management & Full Pipeline
- Study kernel memory allocators (Issue #15)
- Add slab allocator support (Issue #16)
- Evaluate Netlink socket communication (Issue #17)
- Study memory pressure handling (Issue #18)
- Build full integration loop (Issue #19)

## GitHub Repository Management

### Issue & Project Tracking Commands:
```bash
# List all issues
gh api repos/:owner/:repo/issues

# Close an issue with comment
gh issue close <number> --comment "Completed: <description>"

# List GitHub Projects
gh project list --owner <username>

# View project details
gh project view <project_number> --owner <username>

# List project items
gh project item-list <project_number> --owner <username>

# Get project field information (including field IDs)
gh project field-list <project_number> --owner <username>

# Get status field options with IDs
gh api graphql -f query='
  query($owner: String!, $number: Int!) {
    user(login: $owner) {
      projectV2(number: $number) {
        fields(first: 20) {
          nodes {
            ... on ProjectV2SingleSelectField {
              name
              id
              options {
                id
                name
              }
            }
          }
        }
      }
    }
  }' -f owner=<username> -F number=<project_number>

# Update project item status (move between columns)
gh project item-edit --project-id <PROJECT_ID> --id <ITEM_ID> --field-id <FIELD_ID> --single-select-option-id <STATUS_ID>

# Complete workflow example for closing issue and moving to Done:
# 1. Close issue with completion comment
gh issue close 4 --comment "✅ Completed! Implementation available in file.c"

# 2. Move to Done status in project board  
gh project item-edit --project-id PVT_kwHOA1QvYc4A7GKO --id <ITEM_ID> --field-id PVTSSF_lAHOA1QvYc4A7GKOzgvfubI --single-select-option-id 98236657

# Available status options in our project:
# - Backlog (f75ad846)
# - Ready (61e4505c) 
# - In progress (47fc9ee4)
# - In review (df73e18b)
# - Done (98236657)

# Project-specific IDs for kernel-vss-c-refresher (Project #3):
# - Project ID: PVT_kwHOA1QvYc4A7GKO
# - Status Field ID: PVTSSF_lAHOA1QvYc4A7GKOzgvfubI
```
# - In review (df73e18b)
# - Done (98236657)
```

### Project Structure:
- **Project**: "kernel-vss-c-refresher" (Project #3)
- **Repository**: avifenesh/C-refresher
- **Issues**: 19 total, tracking sprint tasks
- **Kanban Board**: C-fast-track-kanban.md with embedded reference links

## Code Examples & Patterns

### Current C Files Context:
- `hello_world.c` - Basic compilation test
- `print_address.c` - Pointer and address manipulation (Issue #1 - COMPLETED)
- `some_simple_types_and_and_pritntf.c` - Basic types and printf
- `sizeofp.c` - Memory size operations
- `increametnt.c` - Basic operations
- `dynamic_array.c` - Dynamic array implementation with malloc/realloc/free (Issue #2 - COMPLETED)
- `128_aligned_vec.c` - 128-bit aligned vector structure with ARM NEON (Issue #4 - COMPLETED)
- `vec_dispatcher.c` - Function pointer callback dispatcher (Issue #5 - IN PROGRESS)
- `LEARNING_PROGRESS.md` - Learning progress tracking file

### Memory Management Patterns:
```c
// Use this pattern for dynamic arrays
typedef struct {
    void **data;
    size_t size;
    size_t capacity;
} dynamic_array_t;

// Always pair malloc with free
void* ptr = malloc(size);
if (!ptr) {
    // Handle allocation failure
    return -ENOMEM;
}
// ... use ptr
free(ptr);
ptr = NULL; // Prevent use-after-free
```

### IOCTL Interface Pattern:
```c
// Kernel IOCTL commands
#define VSS_IOC_MAGIC 'v'
#define VSS_ADD_VECTOR    _IOW(VSS_IOC_MAGIC, 1, struct vector_data)
#define VSS_QUERY_TOP_K   _IOWR(VSS_IOC_MAGIC, 2, struct query_data)
```

## Optimization Guidelines

- Use ARM NEON intrinsics for vector operations: `#include <arm_neon.h>`
- Implement SIMD-optimized cosine similarity: `float32x4_t` for 4-way parallelism
- Consider memory alignment for 128-bit vector operations: `__attribute__((aligned(16)))`
- Use kernel's ring buffer (kfifo) for efficient data structures
- Profile with perf tools for ARM64 optimization opportunities

## Learning Resources Integration

When suggesting code or explaining concepts, reference the embedded links from the Kanban board and provide practical examples that work with the existing ARM64 kernel-style build configuration.

## Learning guidelines

- Don't offer explicit code solutions but provide information needed for understanding
- The goal for the issues and tasks is to practice and learn, any action you take need to be explained and include context and information to give a knowledgable picture
- The only times where explicit code output is allowed are when explicity been asked, or for educetional porpuses, and not for challenge soloving itself.
- Don't assume that a term or a subject is known, for example when using a library, a build flag, an std function, an error message, etc. all need to be explained in detail, unless where already explained or when understanding is clear.
- Create a learning progress markdown file, and fill it during progress with subject learned and example. Keep it informative, and ready for re-reading in case a refresh needed. 

## Error Handling Standards

- Always check return values for system calls and memory allocations
- Use proper kernel error codes (-ENOMEM, -EINVAL, etc.) in kernel code
- Implement graceful cleanup on errors (goto error patterns in kernel code)
- Use Rust's Result types for user-space error propagation
- Include proper logging/debugging output for development phases
