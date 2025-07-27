# 🎓 Learning Linux Kernel Programming Through Text Search

A hands-on learning project to master C systems programming, Linux kernel development, and ARM64 optimization by building a text search engine from scratch.

## 📚 What This Project Is About

This is a **learning-focused project** where I'm teaching myself:
- Linux kernel module development
- Advanced C programming techniques
- Text processing algorithms
- ARM NEON SIMD optimization
- Kernel vs user-space trade-offs

**Important**: This is not a production-ready search engine. It's a structured learning path with practical exercises.

## 🎯 Learning Objectives

### Sprint 1: C Fundamentals ✅
- **What I Learned**: Pointers, memory management, structs, function pointers
- **Key Takeaway**: Solid foundation in C memory model and alignment
- **Status**: COMPLETED - All 6 exercises done

### Sprint 2: Text Processing (Current)
- **What I'm Learning**: How to tokenize text, build inverted indexes, implement TF-IDF
- **Why It Matters**: Understanding data structures and algorithms in C
- **Goal**: Build working text search in user-space first

### Sprint 3: Kernel Module Development
- **What I'll Learn**: Character devices, IOCTL interfaces, kernel memory management
- **Why It Matters**: Understanding kernel/user-space boundaries
- **Challenge**: Moving text processing into kernel space

### Sprint 4: SIMD Optimization
- **What I'll Learn**: ARM NEON intrinsics, vectorization, performance measurement
- **Why It Matters**: Hardware-specific optimization techniques
- **Goal**: Measurable performance improvements

### Sprint 5: Production Considerations
- **What I'll Learn**: Rust FFI, testing strategies, documentation
- **Why It Matters**: Building maintainable systems
- **Outcome**: Understanding full development lifecycle

## 🛠️ Current Progress

```
✅ Sprint 1: Completed all C fundamentals
🔄 Sprint 2: Starting text processing implementation
📅 Sprint 3: Planned - Kernel module development
📅 Sprint 4: Planned - SIMD optimization
📅 Sprint 5: Planned - Production features
```

## 💡 Why This Approach?

1. **Practical Learning**: Each sprint builds on the previous one
2. **Real Challenges**: Kernel programming forces deep understanding
3. **Performance Focus**: SIMD optimization teaches hardware awareness
4. **Incremental Complexity**: Start simple, add complexity gradually

## 🚧 Work in Progress

This repository contains:
- ✅ Completed exercises from Sprint 1
- 🔄 In-progress work for Sprint 2
- 📝 Extensive documentation of what I'm learning
- 🐛 Bugs and failed attempts (part of learning!)
- 📚 Curated resources I'm using to learn

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
- [GeeksforGeeks Inverted Index](https://www.geeksforgeeks.org/inverted-index/) - Text search basics
- [ARM NEON Intrinsics](https://arm-software.github.io/acle/neon_intrinsics/advsimd.html) - SIMD programming
- See [docs/TEXT_SEARCH_RESOURCES.md](docs/TEXT_SEARCH_RESOURCES.md) for complete list

## 🔄 Project Evolution

This project started as a "C refresher" and evolved into a focused kernel programming learning path. The journey:
1. Started with basic C exercises
2. Realized I wanted to learn kernel programming
3. Chose text search as a concrete goal to work towards
4. Structured learning into realistic sprints

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

**Remember**: The goal isn't to build the next Elasticsearch. It's to deeply understand systems programming through hands-on practice.