# 📋 C Fast-Track Learning Path – Kanban (Sprint-Based)

> **Note:** All links verified and updated as of June 2025. Broken links have been replaced with working alternatives.

## 📅 Sprint 1: C Core Refresh & Memory Fundamentals

- **Issue #6** - Setup Makefile with proper flags and GDB integration 📋 **PENDING**
  - Create ARM64-optimized Makefile with kernel-style compilation flags
  - Add GDB debugging symbols and Valgrind memory checking support
  - Generate assembly files for learning and optimization analysis
  - Resources: ([makefile tutorial](https://makefiletutorial.com/), [GDB cheat sheet](https://freecoder.dev/gdb-cheat-sheet/), [valgrind guide](https://valgrind.org/docs/manual/quick-start.html))

## 📅 Sprint 2: System Programming – IOCTL & Char Devices

- **Issue #7** - Build basic char device with miscdevice interface 📋 **BACKLOG**
  - Create kernel module using miscdevice for /dev/vss character device
  - Implement basic file operations (open, close, read, write)
  - Learn miscdevice vs traditional character device registration
  - Resources: ([LDD3](https://lwn.net/Kernel/LDD3/), [LKMPG tutorial](https://tldp.org/LDP/lkmpg/2.6/html/x569.html))

- **Issue #8** - Implement IOCTL protocol (ADD_VECTOR, QUERY_TOP_K) 📋 **BACKLOG**
  - Define IOCTL commands for vector operations using _IOW/_IOWR macros
  - Implement VSS_ADD_VECTOR and VSS_QUERY_TOP_K commands
  - Add proper data structure definitions for kernel-userspace communication
  - Resources: ([kernel IOCTL docs](https://docs.kernel.org/driver-api/ioctl.html), [embetronicx IOCTL tutorial](https://embetronicx.com/tutorials/linux/device-drivers/ioctl-linux-driver-tutorial/))

- **Issue #9** - Build Rust user-space client for /dev/vss communication 📋 **BACKLOG**
  - Create Rust client using nix crate for IOCTL system calls
  - Implement safe wrappers around unsafe IOCTL operations
  - Add proper error handling with Result types
  - Resources: ([nix docs](https://docs.rs/nix/latest/nix/ioctl/index.html), [stackoverflow example](https://stackoverflow.com/questions/59490039/how-to-use-ioctl-in-rust))

- **Issue #10** - Build simple in-kernel vector store (fixed size) 📋 **BACKLOG**
  - Implement fixed-size in-memory vector database in kernel space
  - Use kmalloc for memory allocation with proper error handling
  - Add basic vector storage and retrieval operations
  - Resources: ([LDD3 memory chapters](https://lwn.net/Kernel/LDD3/))

## 📅 Sprint 3: Performance Structures – Ring Buffers & SIMD

- **Issue #11** - Implement ring buffer with FIFO eviction 📋 **BACKLOG**
  - Create circular buffer implementation using kernel kfifo or custom solution
  - Implement FIFO (First In, First Out) eviction strategy for vector storage
  - Add thread-safe operations for concurrent access in kernel space
  - Resources: ([circular buffer guide](https://www.embedded.com/c-language-techniques-for-circular-buffers/), [kernel kfifo docs](https://www.kernel.org/doc/html/latest/core-api/kfifo.html))

- **Issue #12** - Study LRU cache eviction strategies 📋 **BACKLOG**
  - Learn Least Recently Used (LRU) algorithm implementation
  - Compare LRU vs FIFO performance characteristics for vector databases
  - Design data structures for efficient LRU tracking
  - Resources: ([linux-mm docs](https://linux-mm.org/PageReplacementAlgorithms), [LRU implementation](https://gist.github.com/marcosfelt/7974411))

- **Issue #13** - Implement basic cosine similarity in C 📋 **BACKLOG**
  - Create dot product and magnitude calculation functions
  - Implement cosine similarity algorithm for vector comparison
  - Add proper handling for edge cases (zero vectors, normalization)
  - Resources: ([similarity guide](https://www.geeksforgeeks.org/program-find-similarity-two-strings/))

- **Issue #14** - Implement cosine similarity with NEON SIMD intrinsics 📋 **BACKLOG**
  - Optimize cosine similarity using ARM NEON SIMD instructions
  - Use float32x4_t for 4-way parallelism on 128-bit vectors
  - Profile performance improvements over scalar implementation
  - Resources: ([ARM NEON docs](https://developer.arm.com/documentation/den0018/a/NEON-Intrinsics/), [NEON intrinsics guide](https://documentation-service.arm.com/static/67a384f0e80ab6345cace1af), [GitHub NEON guide](https://github.com/thenifty/neon-guide))

## 📅 Sprint 4: Kernel Memory Management & Full Pipeline

- **Issue #15** - Study kernel memory allocators (kmalloc, slab) 📋 **BACKLOG**
  - Learn kernel memory allocation strategies and best practices
  - Understand kmalloc vs vmalloc vs get_free_pages differences
  - Study memory zones and allocation flags (GFP_KERNEL, GFP_ATOMIC, etc.)
  - Resources: ([LDD3 reference](https://lwn.net/Kernel/LDD3/), [kernel memory docs](https://www.kernel.org/doc/html/latest/core-api/kmem.html))

- **Issue #16** - Add slab allocator support (optional) 📋 **BACKLOG**
  - Implement custom slab cache for frequently allocated vector objects
  - Compare performance benefits vs standard kmalloc approach
  - Add proper cache initialization and cleanup in module init/exit
  - Resources: ([kernel slab docs](https://www.kernel.org/doc/html/latest/core-api/kmem.html))

- **Issue #17** - Evaluate Netlink socket communication (optional) 📋 **BACKLOG**
  - Compare Netlink sockets vs IOCTL for kernel-userspace communication
  - Implement basic Netlink protocol for vector operations
  - Analyze performance and complexity trade-offs
  - Resources: ([GitHub netlink examples](https://github.com/ANSANJAY/NetlinkGreetingExchange), [netlink socket tutorial](https://github.com/apofiget/netlink-example))

- **Issue #18** - Study memory pressure handling and hooks (optional advanced) 📋 **BACKLOG**
  - Learn kernel memory pressure detection and response mechanisms
  - Implement graceful degradation when system memory is low
  - Add memory accounting and monitoring for vector database
  - Resources: ([kernel memory management](https://docs.kernel.org/admin-guide/mm/index.html), [memory accounting](https://docs.kernel.org/accounting/memory.html))

- **Issue #19** - Build full integration loop 📋 **BACKLOG**
  - Create end-to-end pipeline: Rust client → kernel IOCTL → cosine similarity → return IDs
  - Integrate with external Valkey database for object storage
  - Add comprehensive testing and performance benchmarking
  - Resources: ([valkey docs](https://valkey.io/docs/reference/client-libraries/))

## 🚧 In Progress

- Learn and practice function pointers (callback dispatcher) ([guide](https://www.geeksforgeeks.org/function-pointer-in-c/))

## ✅ Done

- Review pointer operations in C ([guide](https://beej.us/guide/bgc/html/split/pointers.html))

- Build dynamic array (malloc, realloc, free) ([tutorial](https://www.learn-c.org/en/Dynamic_memory))

- Study memory alignment and struct padding ([reference](https://www.geeksforgeeks.org/data-structure-alignment-in-c/))

- Build aligned struct for 128-bit vectors with NEON SIMD support ([code](128_aligned_vec.c))

