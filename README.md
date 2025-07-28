# 🎓 ARM-Optimized Kernel Storage Engine Comparison

A hands-on learning project to master C systems programming, Linux kernel development, and ARM64 optimization by building multiple storage engines from scratch and comparing their performance.

## 📚 What This Project Is About

This is a **learning-focused project** where I'm teaching myself:
- Linux kernel module development
- Advanced C programming techniques  
- Storage engine algorithms and trade-offs
- ARM NEON SIMD optimization
- Kernel vs user-space trade-offs

**Important**: This is not a production-ready database system. It's a structured learning path with practical exercises focused on storage engine implementation.

## 🎯 Learning Objectives

### Sprint 1: C Fundamentals ✅
- **What I Learned**: Pointers, memory management, structs, function pointers
- **Key Takeaway**: Solid foundation in C memory model and alignment
- **Status**: COMPLETED - All 6 exercises done

### Sprint 2: Hash Table Storage Engine (Current)
- **What I'm Learning**: Hash table algorithms, collision resolution, kernel character devices
- **Why It Matters**: Understanding fundamental database storage algorithms in C
- **Goal**: Build `/dev/storage-hash` with IOCTL interface and basic operations

### Sprint 3: B+ Tree Storage Engine
- **What I'll Learn**: B+ tree algorithms, sorted data structures, kernel memory management
- **Why It Matters**: Understanding indexed storage for range queries
- **Challenge**: Implementing efficient tree operations in kernel space

### Sprint 4: LSM Tree Storage Engine  
- **What I'll Learn**: LSM tree design, write optimization, background compaction
- **Why It Matters**: Modern database storage architecture
- **Goal**: Write-optimized storage with background kernel threads

### Sprint 5: Performance Analysis & Production Features
- **What I'll Learn**: Benchmarking, Rust FFI, ARM NEON optimization
- **Why It Matters**: Real-world database performance comparison
- **Outcome**: Compare against SQLite/LMDB on Raspberry Pi hardware

## 🛠️ Current Progress

```
✅ Sprint 1: Completed all C fundamentals
🔄 Sprint 2: Starting hash table storage engine implementation
📅 Sprint 3: Planned - B+ tree storage engine
📅 Sprint 4: Planned - LSM tree storage engine
📅 Sprint 5: Planned - Performance analysis and ARM optimization
```

## 💡 Why This Approach?

1. **Practical Learning**: Each storage engine builds on previous algorithms
2. **Real Challenges**: Kernel programming forces deep understanding of system internals
3. **Performance Focus**: ARM NEON optimization teaches hardware-specific database tuning
4. **Incremental Complexity**: Hash tables → B+ trees → LSM trees (increasing complexity)
5. **Real Comparison**: Benchmarking against production databases (SQLite, LMDB)

## 🚧 Work in Progress

This repository contains:
- ✅ Completed exercises from Sprint 1  
- 🔄 In-progress work for Sprint 2 (hash storage engine)
- 📝 Extensive documentation of storage engine algorithms
- 🐛 Bugs and failed attempts (part of learning!)
- 📚 Curated database systems resources

## ⚠️ Important Notes

### This is a Learning Project
- Code may be incomplete or buggy
- Approaches may not be optimal (yet)
- Documentation reflects my understanding at the time
- Mistakes are part of the learning process

### Kernel Development Warnings
- Sprint 3 involves kernel modules that can crash systems
- Always use a VM for kernel development
- I'm learning safety practices as I go

## 📖 Learning Resources I'm Using

- [Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/) - Modern kernel development
- [Database Internals](https://www.databass.dev/) - Storage engine algorithms and theory
- [ARM NEON Intrinsics](https://arm-software.github.io/acle/neon_intrinsics/advsimd.html) - SIMD programming
- See [docs/STORAGE_ENGINE_RESOURCES.md](docs/STORAGE_ENGINE_RESOURCES.md) for complete list

## 🔄 Project Evolution

This project started as a "C refresher" and evolved into a focused kernel programming learning path. The journey:
1. Started with basic C exercises
2. Realized I wanted to learn kernel programming  
3. Initially chose text search, then pivoted to storage engines for better database expertise leverage
4. Structured learning into realistic sprints focused on different storage algorithms

## 🤝 Learning Together

If you're also learning:
- Check out the [detailed documentation](docs/) for each sprint
- Look at completed exercises in `sprint-1-core-memory/`
- Follow along with the issues and sprint plans
- Learn from my mistakes in the commit history!

## 📝 Notes

- **Timeline**: Expecting 3-5 months to complete all sprints
- **Platform**: Targeting ARM64 (Raspberry Pi or similar)
- **Background**: Assuming C knowledge but learning kernel programming from scratch

---

**Remember**: The goal isn't to build the next PostgreSQL. It's to deeply understand database systems and kernel programming through hands-on storage engine implementation.