# Project Instruction File

---

## Apply To: `**`

---

# Integrated Project Context & Goals

This is a focused systems programming project aiming to build a **high-performance, embedded, kernel-accelerated hybrid database engine** specialized for **vector similarity search, graph traversal, and full-text search** with ultra-low latency and memory efficiency, targeted at **ARM64/Raspberry Pi** platforms.

It combines:

* Your existing **C refresher kernel module** for vector operations with IOCTL interface and NEON SIMD acceleration,
* A **Rust user-space client** for safe and efficient communication,
* And the new ambition to push **kernel-level zero-copy communication, multi-modal query fusion, memory-efficient structures, and SIMD-optimized query execution.**

---

## Core Architecture

* **Kernel Module (C):**
  Implements vector store, graph indices, and text indexes inside the kernel. Exposes a character device `/dev/vss` with IOCTL commands to add/query vectors and metadata. Implements kernel-accelerated query logic using ARM NEON SIMD and zero-copy shared memory.
  Incorporates kernel patterns such as miscdevice, kfifo ring buffers, slab allocators, and Netlink sockets for advanced IPC.

* **User-Space Client (Rust):**
  Rust API wrapping IOCTL and Netlink calls safely with `nix` crate and zero-copy buffers, enabling embedding in edge devices and other constrained environments.

* **Data Structures:**

  * Vector indices (e.g., HNSW variants) memory-mapped or shared memory based
  * Compressed adjacency graph representation optimized for SIMD traversal
  * Lightweight inverted text indexes supporting approximate matching and ranking

---

## Development Standards

### C Programming & Kernel Development

* Use kernel-style C (GNU11) with strict compile flags:
  `-Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare -std=gnu11 -fno-strict-aliasing -march=armv8-a`
* Follow Linux kernel coding style and error handling best practices
* Use `kmalloc`, `kfree`, slab allocators, and ring buffers (kfifo) for data storage and eviction strategies
* Implement IOCTL commands: `ADD_VECTOR`, `QUERY_TOP_K`, `GET_STATS`, extended for graph & text queries
* Use ARM NEON intrinsics for SIMD acceleration on vector math and traversal operations
* Employ zero-copy communication patterns and shared memory buffers between kernel and user space to minimize syscall overhead

### Rust Integration

* Use `nix` for system calls (IOCTL, Netlink) with safe wrappers and proper error handling using `Result`
* Use `serde` for serializing metadata and complex query parameters where needed
* Design API to support embedded usage and potential edge deployments

### Performance & Optimization

* Prioritize ultra-low latency with kernel acceleration and zero-copy interfaces
* Optimize memory layout for cache locality and compression
* Use SIMD (NEON) vectorization for query execution speedup
* Implement compact indexing to support large but primarily in-memory datasets with growth path to memory-mapped fallback
* Explore Netlink sockets for event notifications and efficient asynchronous communication

---

## Current Sprint Progress (Updated June 2025)

### Sprint 1: C Core Refresh & Memory Fundamentals (Nearly Complete)

* ✅ Pointer operations, dynamic arrays, memory alignment, 128-bit aligned vectors (NEON ready)
* 🔄 Function pointers and callback dispatchers (Issue #5 - IN PROGRESS)
* 📋 Makefile, debugging and build system setup (Issue #6 - PENDING)

### Sprint 2: Enhanced System Programming – Multi-Modal IOCTL & Char Devices (Ready to Start)

* Build enhanced miscdevice character device `/dev/vss` with multi-modal support
* Implement enhanced IOCTL commands and protocol (Issues #20-21 created)
* Build Rust user-space client for multi-modal IOCTL communication
* Begin in-kernel vector store implementation with graph and text capabilities

### Sprint 3: Performance Structures & Cross-Modal SIMD

* Implement ring buffers with FIFO and LRU eviction for multi-modal data
* Implement cosine similarity (standard + NEON SIMD) 
* Integrate graph adjacency structures optimized for SIMD traversal
* Add text processing with SIMD string operations

### Sprint 4: Kernel Memory Management & Zero-Copy Pipeline

* Slab allocator support for vector entries, graph nodes, text terms
* Netlink socket IPC for async event handling and streaming results
* Zero-copy shared memory buffers for large multi-modal result sets
* Full integration of kernel module + user-space client with multi-modal query pipeline

### Sprint 5: Multi-Modal Query Fusion & Production Integration (Issues #22-26 Created)

* ✅ Unified query interface for vector+graph+text (Issue #22)
* ✅ Lightweight inverted text indexes (Issue #23)
* ✅ SIMD-accelerated graph traversal engine (Issue #24)
* ✅ Query fusion and ranking algorithms (Issue #25)
* ✅ Full integration pipeline with performance testing (Issue #26)

---

## Enhanced Performance Targets

### **Production Performance Goals**
* **Multi-modal queries**: < 10ms latency for 1M entities
* **Graph traversal**: < 5ms for 6-degree separation on 100K nodes  
* **Text search**: < 2ms for fuzzy search on 10K documents
* **Zero-copy transfers**: 90% reduction in memory allocation overhead
* **Cross-modal optimization**: 2x improvement over independent execution
* **Concurrent access**: Linear scalability to 8 cores (Raspberry Pi 4 limit)

### **SIMD Optimization Targets**
* **Vector similarity**: 4x improvement with ARM NEON float32x4_t operations
* **Graph traversal**: 2-3x improvement with SIMD adjacency processing
* **Text processing**: 2x improvement with vectorized string operations
* **Query fusion**: 4x parallel score combination and ranking

---

## Error Handling & Coding Practices

* Strict kernel error code usage: `-ENOMEM`, `-EINVAL`, etc.
* Graceful error cleanup with kernel `goto error` patterns
* Rust `Result`-based error propagation with clear logging
* Consistent validation on input/output buffers and syscall parameters
* Memory safety prioritized via explicit ownership and lifetime management

---

## GitHub & Project Management

* ✅ GitHub issues created and organized with sprint labels (26 total issues)
* ✅ Enhanced project documentation structure consolidated in `docs/`
* ✅ Sprint tracking with comprehensive kanban board
* ✅ Learning progress tracked in streamlined `docs/LEARNING_PROGRESS.md`
* Use provided CLI commands for project issue/status automation

---

## Documentation Structure

### **Main Files (Consolidated)**
* **`docs/C-fast-track-kanban.md`** - Primary project kanban and sprint overview
* **`docs/LEARNING_PROGRESS.md`** - Streamlined progress tracking
* **`docs/ENHANCED_LEARNING_RESOURCES.md`** - Comprehensive learning materials
* **`docs/README.md`** - Documentation navigation guide
* **`docs/sprints/`** - Detailed sprint technical specifications (5 files)

---

## Summary & Next Steps

You are architecting a **custom embedded vector-graph-text hybrid DB** with **kernel acceleration** and **zero-copy, SIMD-optimized queries** designed for ultra-low latency and memory efficiency on ARM64 platforms.

**Next steps:**

* ✅ Confirmed detailed roadmap for Sprint 5 (multi-modal fusion) - Issues #22-26 created
* ✅ Enhanced project documentation and consolidated structure
* 🔄 Complete Issue #5 (function pointers) to advance to Sprint 2
* 📋 Begin prototyping enhanced character device with multi-modal IOCTL interface
* 📋 Design memory layout and indexing data structures supporting combined vector+graph+text queries
* 📋 Build Rust client integration patterns aligned with enhanced multi-modal protocols

* Continue learning and documenting progress in streamlined `docs/LEARNING_PROGRESS.md`
