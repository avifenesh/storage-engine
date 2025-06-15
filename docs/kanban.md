# 📋 C Fast-Track Learning Path – Kanban (Sprint-Based)

## 📅 Sprint 1: C Core Refresh & Memory Fundamentals

- **Issue #6** - Setup Makefile with proper flags and GDB integration 📋 **PENDING**

## 📅 Sprint 2: System Programming – IOCTL & Char Devices

- **Issue #7** - Build basic char device with miscdevice interface 📋 **BACKLOG**

- **Issue #8** - Implement IOCTL protocol (ADD_VECTOR, QUERY_TOP_K) 📋 **BACKLOG**

- **Issue #9** - Build Rust user-space client for /dev/vss communication 📋 **BACKLOG**

- **Issue #10** - Build simple in-kernel vector store (fixed size) 📋 **BACKLOG**

## 📅 Sprint 3: Performance Structures – Ring Buffers & SIMD

- **Issue #11** - Implement ring buffer with FIFO eviction 📋 **BACKLOG**

- **Issue #12** - Study LRU cache eviction strategies 📋 **BACKLOG**

- **Issue #13** - Implement basic cosine similarity in C 📋 **BACKLOG**

- **Issue #14** - Implement cosine similarity with NEON SIMD intrinsics 📋 **BACKLOG**

## 📅 Sprint 4: Kernel Memory Management & Full Pipeline

- **Issue #15** - Study kernel memory allocators (kmalloc, slab) 📋 **BACKLOG**

- **Issue #16** - Add slab allocator support (optional) 📋 **BACKLOG**

- **Issue #17** - Evaluate Netlink socket communication (optional) 📋 **BACKLOG**

- **Issue #18** - Study memory pressure handling and hooks (optional advanced) 📋 **BACKLOG**

- **Issue #19** - Build full integration loop 📋 **BACKLOG**

## 🚧 In Progress

- **Issue #5** - Learn function pointers for callback dispatchers 🔄 **IN PROGRESS**

## ✅ Done

- **Issue #1** - Review pointer operations in C ✅ **COMPLETED**

- **Issue #2** - Build dynamic array (malloc, realloc, free) ✅ **COMPLETED**

- **Issue #3** - Study memory alignment and struct padding ✅ **COMPLETED**

- **Issue #4** - Build aligned struct for 128-bit vectors ✅ **COMPLETED**

## 📋 Project Overview

- **Kernel Module (C)**: In-kernel vector database with IOCTL interface, optimized with ARM NEON SIMD

- **User-space Client (Rust)**: Communicates with kernel via IOCTL commands (`/dev/vss`)

- **Memory Store**: In-memory database containing actual objects that vectors represent

- **Platform**: ARM64/Raspberry Pi with kernel programming focus

- **Ring Buffer vs Linear Search:** 10-100x improvement for cache operations

- **NEON vs Scalar:** 3-4x improvement for vector similarity

- **Combined Optimizations:** 50-200x improvement for top-k queries

## 📚 Documentation Structure

- Structure
    ```md
    ```
    docs/
    ├── C-fast-track-kanban.md          # This file - Sprint overview
    ├── LEARNING_PROGRESS.md            # Progress tracking and notes
    └── sprints/
        ├── sprint-1-c-core-memory.md   # C fundamentals & memory management
        ├── sprint-2-system-programming.md  # IOCTL & character devices
        ├── sprint-3-performance-simd.md    # Ring buffers & NEON SIMD
        └── sprint-4-kernel-memory.md       # Advanced memory & integration
    ```
    ```

## 🎯 Quick Links

- [GitHub Project Board](https://github.com/users/avifenesh/projects/3)

- [Repository Issues](https://github.com/avifenesh/C-refresher/issues)

- [Learning Progress](LEARNING_PROGRESS.md)

- [Project Instructions](.github/instructions/instructions.instructions.md)

