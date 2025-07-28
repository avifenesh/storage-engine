# 📚 Storage Engine Development - Learning Resources

## Overview
Comprehensive collection of the BEST learning resources for building ARM-optimized kernel storage engines, including video tutorials, interactive courses, and practical guides for database system implementation.

## 🎥 **BEST VIDEO LEARNING RESOURCES** 🎥

### Top Video Tutorials and Courses
Based on extensive research, these represent the highest-quality video content for our learning objectives:

---

## 📖 Core Database Systems & Storage Engines

### Storage Engine Fundamentals
- **[Database Internals](https://www.databass.dev/)** - Comprehensive guide to storage engine algorithms
- **[Hash Tables in C](https://benhoyt.com/writings/hash-table-in-c/)** - Essential for building efficient hash storage
- **[B+ Tree Implementation Guide](https://github.com/begeekmyfriend/bplustree)** - Practical B+ tree in C
- **[LSM Tree Explained](https://github.com/facebook/rocksdb/wiki/LSM-Tree)** - Write-optimized storage design

### Database Algorithm Deep Dives
- **[Understanding B+ Trees](https://www.geeksforgeeks.org/introduction-of-b-tree/)** - Sorted storage for range queries
- **[LSM Tree Design](https://www.cs.umb.edu/~poneil/lsmtree.pdf)** - Original LSM tree paper
- **[Hash Table Collision Resolution](https://en.wikipedia.org/wiki/Hash_table#Collision_resolution)** - Chaining vs open addressing
- **[Database Storage Engines Comparison](https://github.com/pingcap/awesome-database-learning)** - Learning database internals

### Production Storage Engine Analysis
- **[SQLite Architecture](https://www.sqlite.org/arch.html)** - Simple, efficient embedded database design
- **[LMDB Design](https://github.com/LMDB/lmdb)** - Memory-mapped B+ tree storage
- **[RocksDB Architecture](https://github.com/facebook/rocksdb/wiki/Architecture-Guide)** - LSM tree implementation
- **[Valkey Storage Engine](https://github.com/valkey-io/valkey)** - Modern Redis fork storage patterns

---

## 🐧 Linux Kernel Development

### ⚠️ IMPORTANT SAFETY WARNING
Kernel development can crash your system. Always:
- Use a virtual machine or dedicated test system
- Never test on production systems
- Keep backups of important data
- Understand that kernel bugs can corrupt filesystems

### 🎥 **TOP VIDEO TUTORIALS**
- **[freeCodeCamp Linux Device Driver Development (5+ hours)](https://www.youtube.com/watch?v=juGNPLdjLH4)** - ⭐ BEST comprehensive video course
  - Created by Piyush Itankar (Google embedded systems engineer)
  - Covers character devices, IOCTL, kernel modules
  - Hands-on examples with real hardware
- **[Johannes4Linux Driver Tutorial Series](https://github.com/Johannes4Linux/Linux_Driver_Tutorial)** - ⭐ 2024 modern tutorial
  - YouTube playlist with GitHub code examples
  - Raspberry Pi focused with practical exercises
  - Started in 2024, actively maintained

### Essential Books and Guides
- **[The Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)** - ⭐ BEST for beginners (Updated July 27, 2025)
- **[Linux Device Drivers, 3rd Edition](https://lwn.net/Kernel/LDD3/)** - ⚠️ OUTDATED (2005) but concepts still relevant
- **[Bootlin Kernel Training Materials](https://bootlin.com/doc/training/linux-kernel/)** - Free, comprehensive slides

### Character Device Development
- **[Writing a Simple Character Device](https://embetronicx.com/tutorials/linux/device-drivers/linux-device-driver-tutorial-part-2-first-device-driver/)** - Step-by-step tutorial
- **[Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)** - Updated for recent kernels
- **[Miscdevice Interface](https://www.kernel.org/doc/html/latest/driver-api/misc_devices.html)** - Simpler than full char devices

### IOCTL Interface Design
- **[IOCTL Tutorial](https://embetronicx.com/tutorials/linux/device-drivers/ioctl-tutorial-in-linux/)** - Comprehensive IOCTL guide
- **[Kernel IOCTL Documentation](https://www.kernel.org/doc/html/latest/driver-api/ioctl.html)** - Official kernel docs
- **[IOCTL Best Practices](https://stackoverflow.com/questions/15807846/ioctl-linux-device-driver)** - Common patterns for database operations

### Kernel Memory Management
- **[Kernel Memory Allocation](https://www.kernel.org/doc/html/latest/core-api/memory-allocation.html)** - kmalloc, kfree for database structures
- **[Linux Memory Management](https://www.kernel.org/doc/gorman/html/understand/)** - Deep dive into kernel memory
- **[Kernel Hash Tables](https://lwn.net/Articles/510202/)** - Using kernel hash tables for storage
- **[Memory Barriers and Locking](https://www.kernel.org/doc/html/latest/core-api/memory-hotplug.html)** - Concurrent access patterns

### Kernel Debugging Techniques
- **[printk and dmesg](https://www.kernel.org/doc/html/latest/core-api/printk-basics.html)** - Basic kernel debugging
- **[Dynamic Debug (dyndbg)](https://www.kernel.org/doc/html/latest/admin-guide/dynamic-debug-howto.html)** - Runtime debug control
- **[KGDB Setup Guide](https://www.kernel.org/doc/html/latest/dev-tools/kgdb.html)** - Source-level kernel debugging
- **[ftrace Tutorial](https://www.kernel.org/doc/html/latest/trace/ftrace.html)** - Database operation tracing
- **[kmemleak](https://www.kernel.org/doc/html/latest/dev-tools/kmemleak.html)** - Memory leak detection for storage structures
- **[KASAN](https://www.kernel.org/doc/html/latest/dev-tools/kasan.html)** - Address sanitizer for kernel data structures

---

## 💪 ARM NEON SIMD Optimization

### 🎥 **TOP ARM NEON VIDEO RESOURCES**

**Note**: Specific YouTube tutorials for ARM NEON SIMD programming from 2024-2025 are limited. The following represent the best available learning resources:

- **[ARM Developer Official Documentation](https://developer.arm.com/documentation/102474/latest/)** - ⭐ BEST comprehensive guide
  - Learn the architecture - Introducing Neon
  - SIMD ISAs and Neon Intrinsics Reference
  - Interactive examples and code samples
- **[CppCon ARM NEON Talks](https://www.youtube.com/results?search_query=CppCon+ARM+NEON)** - Search for conference presentations
- **[Linux Magazine SIMD Code Optimization (2024)](https://www.linux-magazine.com/Issues/2024/284/SIMD-Code-Optimization)** - Recent practical guide

### Essential NEON Resources
- **[ARM NEON Intrinsics Reference](https://arm-software.github.io/acle/neon_intrinsics/advsimd.html)** - ⭐ Official maintained reference
- **[Practical NEON Tutorial](https://www.codeproject.com/Articles/5294663/Neon-Intrinsics-for-Optimized-Math-Networking-and)** - Real-world examples
- **[Mozilla SIMD Assembly Tutorial](https://silo.tips/download/simd-assembly-tutorial-arm-neon-mozilla)** - Deep dive into assembly
- **[ARM Assembly by Example - NEON](https://armasm.com/docs/neon/overview/)** - Practical assembly examples

### Database Operations with NEON
- **[SIMD Hash Functions](https://lemire.me/blog/2021/01/29/arm-neon-programming-quick-reference/)** - Optimized hashing
- **[Vectorized Key Comparison](https://arxiv.org/abs/1612.01506)** - SIMD string comparison for B+ trees
- **[NEON Optimization Examples](https://github.com/DLTcollab/sse2neon)** - SSE to NEON conversions
- **[Fast Memory Operations](https://developer.arm.com/documentation/102564/latest)** - NEON for data movement

### Performance Measurement
- **[ARM Performance Monitoring](https://developer.arm.com/documentation/102450/latest)** - Using PMU counters
- **[Linux perf on ARM](https://www.kernel.org/doc/html/latest/arm64/perf.html)** - Database profiling on ARM64
- **[Benchmarking Database Operations](https://github.com/brianfrankcooper/YCSB)** - Standard database benchmarks

---

## 🦀 Rust Client Library Development

### Rust Basics for C Developers
- **[The Rust Book](https://doc.rust-lang.org/book/)** - Official Rust tutorial
- **[Rust for C++ Programmers](https://github.com/nrc/r4cppp)** - Transition guide
- **[Rust FFI Guide](https://doc.rust-lang.org/nomicon/ffi.html)** - Interfacing with C storage engines
- **[The Rust FFI Omnibus](http://jakegoulding.com/rust-ffi-omnibus/)** - Practical FFI examples

### Linux System Programming in Rust
- **[nix crate documentation](https://docs.rs/nix/latest/nix/)** - Unix/Linux APIs in Rust
- **[IOCTL in Rust](https://docs.rs/nix/latest/nix/sys/ioctl/index.html)** - Safe IOCTL wrappers for storage engines
- **[Rust Linux Examples](https://github.com/rust-lang/libc)** - System call examples
- **[Database Clients in Rust](https://github.com/diesel-rs/diesel)** - Modern database client patterns

---

## 🔍 Database Storage Engine Implementation

### 🎥 **TOP DATABASE ENGINE VIDEO RESOURCES**

**Database Internals and Storage Engines**:
- **[CMU Database Systems Course (15-445/645)](https://15445.courses.cs.cmu.edu/)** - ⭐ BEST university course
  - Full video lectures available on YouTube
  - Covers storage engine implementation in detail
  - Taught by Andy Pavlo, world-class database systems expert
- **[Build a Database From Scratch Videos](https://www.youtube.com/results?search_query="build+database+from+scratch")** - Search for practical tutorials

**Written Resources (Highly Recommended)**:
- **[LSM in a Week](https://skyzh.github.io/mini-lsm/)** - ⭐ BEST hands-on LSM tree course (2024)
  - Complete Rust implementation tutorial
  - Free forever with lifetime updates
  - Discord community support
- **[Building LSM-Tree Storage Engine from Scratch](https://dev.to/justlorain/building-an-lsm-tree-storage-engine-from-scratch-3eom)** - 2025 tutorial
- **[Database From Scratch with Python](https://hackernoon.com/how-to-build-a-database-from-scratch-understanding-lsm-trees-and-storage-engines-part-1)** (Oct 2024)

### Open Source References
- **[SQLite Storage Engine](https://www.sqlite.org/fileformat.html)** - B+ tree implementation details
- **[LMDB Source Code](https://github.com/LMDB/lmdb)** - Memory-mapped B+ tree
- **[RocksDB Storage](https://github.com/facebook/rocksdb)** - Production LSM tree
- **[TiKV B-Tree vs LSM-Tree](https://tikv.org/deep-dive/key-value-engine/b-tree-vs-lsm/)** - ⭐ Excellent comparison

### Implementation Tutorials
- **[Build a Database in C](https://cstack.github.io/db_tutorial/)** - Step-by-step SQLite clone
- **[Hash Table Tutorial](https://github.com/jamesroutley/write-a-hash-table)** - Clean hash table implementation
- **[B+ Tree Implementation Guide](https://github.com/begeekmyfriend/bplustree)** - Practical C implementation

### Performance Optimization
- **[Database Performance](https://use-the-index-luke.com/)** - Index and query optimization
- **[Cache-Friendly Data Structures](https://www.intel.com/content/www/us/en/developer/articles/technical/cache-blocking-techniques.html)** - Intel's guide
- **[Linux Performance Tools](https://www.brendangregg.com/linuxperf.html)** - Brendan Gregg's database profiling toolkit

---

## 📝 **OPTIMIZED PROJECT LEARNING PATH**

### 🎯 Sprint 2: Hash Table Storage Engine (3-4 weeks)
**Video Learning**:
1. **[freeCodeCamp Linux Device Driver Course](https://www.youtube.com/watch?v=juGNPLdjLH4)** - Character device fundamentals
2. **[Johannes4Linux Tutorial Playlist](https://github.com/Johannes4Linux/Linux_Driver_Tutorial)** - Modern examples

**Practical Implementation**:
1. Start with [Hash Tables in C](https://benhoyt.com/writings/hash-table-in-c/)
2. Build character device following [EmbeTronicX IOCTL tutorial](https://embetronicx.com/tutorials/linux/device-drivers/ioctl-tutorial-in-linux/)
3. Add ARM NEON hash optimization using [ARM Developer Guide](https://developer.arm.com/documentation/102474/latest/)

### 🎯 Sprint 3: B+ Tree Storage Engine (3-4 weeks)
**Video Learning**:
1. **[CMU Database Systems Lectures](https://15445.courses.cs.cmu.edu/)** - Storage and indexing
2. Search YouTube for "B+ tree implementation tutorial"

**Practical Implementation**:
1. Study [B+ tree algorithms](https://www.geeksforgeeks.org/introduction-of-b-tree/)
2. Review [practical C implementation](https://github.com/begeekmyfriend/bplustree)
3. Add NEON key comparison using [vectorized techniques](https://arxiv.org/abs/1612.01506)

### 🎯 Sprint 4: LSM Tree Storage Engine (3-4 weeks)
**Interactive Learning**:
1. **[LSM in a Week Course](https://skyzh.github.io/mini-lsm/)** - ⭐ BEST hands-on tutorial
2. **[Building LSM from Scratch](https://dev.to/justlorain/building-an-lsm-tree-storage-engine-from-scratch-3eom)** - 2025 guide

**Advanced Topics**:
1. Study [LSM tree theory](https://www.cs.umb.edu/~poneil/lsmtree.pdf)
2. Analyze [RocksDB architecture](https://github.com/facebook/rocksdb/wiki/Architecture-Guide)
3. Implement compaction using [kernel work queues](https://www.kernel.org/doc/html/latest/core-api/workqueue.html)

### 🎯 Sprint 5: Production Features (3-4 weeks)
**Systems Design**:
1. Study [Raft consensus algorithm](https://raft.github.io/raft.pdf)
2. Learn from [Designing Data-Intensive Applications](https://dataintensive.net/)
3. Profile using [ARM PMU counters](https://developer.arm.com/documentation/102450/latest)

---

## 🛠️ Development Tools

### Testing Frameworks
- **[Unity Test Framework](https://github.com/ThrowTheSwitch/Unity)** - Lightweight C testing for storage engines
- **[CUnit](http://cunit.sourceforge.net/)** - Traditional xUnit-style testing
- **[Kernel Testing Guide](https://www.kernel.org/doc/html/latest/dev-tools/testing-overview.html)** - Testing kernel storage modules

### Debugging
- **[GDB for Kernel Debugging](https://www.kernel.org/doc/html/latest/dev-tools/gdb-kernel-debugging.html)** - Debug storage engine kernel code
- **[Valgrind on ARM](https://valgrind.org/docs/manual/manual-core.html)** - Memory debugging for user-space tests
- **[AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer)** - Built into GCC/Clang

### Performance Analysis
- **[perf Tutorial](https://perf.wiki.kernel.org/index.php/Tutorial)** - Database operation profiling
- **[Flamegraphs](https://www.brendangregg.com/flamegraphs.html)** - Visualizing storage engine performance
- **[ARM Streamline](https://developer.arm.com/tools-and-software/graphics-and-gaming/arm-mobile-studio/components/streamline-performance-analyzer)** - ARM-specific database profiling

---

## 📚 Additional Reading

### Classic Database Papers
- **[The Design and Implementation of a Log-Structured File System](https://people.eecs.berkeley.edu/~brewer/cs262/LFS.pdf)** - Foundation for LSM trees
- **[Modern B-Tree Techniques](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/bw-tree-icde2013-final.pdf)** - Advanced indexing
- **[Architecture of a Database System](http://db.cs.berkeley.edu/papers/fntdb07-architecture.pdf)** - Database system overview

### 🎓 **BEST ONLINE COURSES**
- **[CMU Database Systems (15-445/645)](https://15445.courses.cs.cmu.edu/)** - ⭐ BEST for storage engines
  - Full video lectures on YouTube
  - Homework assignments and projects available
  - Taught by Andy Pavlo, renowned database expert
- **[MIT Operating Systems (6.828)](https://pdos.csail.mit.edu/6.828/)** - ⭐ BEST for kernel programming
  - Hands-on xv6 operating system development
  - Deep dive into system calls and kernel internals
- **[Linux Foundation Kernel Development (LFD103)](https://training.linuxfoundation.org/training/a-beginners-guide-to-linux-kernel-development-lfd103/)** - Beginner-friendly
- **[CS50 Introduction to Computer Science](https://cs50.harvard.edu/)** - Strong C fundamentals

---

## 🎯 **LEARNING STRATEGY RECOMMENDATIONS**

### Video-First Approach
1. **Start with videos** - Watch the freeCodeCamp Linux Driver course for foundational understanding
2. **Follow along with code** - Use Johannes4Linux tutorials for hands-on practice
3. **Deep dive with courses** - Take CMU Database Systems course for storage engine theory
4. **Build incrementally** - Follow the LSM in a Week course for practical implementation

### Optimal Learning Path
- **Kernel Basics**: freeCodeCamp → Johannes4Linux → Linux Kernel Programming Guide
- **Storage Engines**: CMU Course → LSM in a Week → Build a Database in C
- **SIMD Optimization**: ARM Developer Docs → Mozilla SIMD Tutorial → Practical examples
- **Production Systems**: Raft Paper → Designing Data-Intensive Applications → SRE Book

---

*Focus on understanding and implementing core storage algorithms through hands-on video tutorials and interactive courses. The goal is to build working storage engines and learn database systems through practical kernel programming.*