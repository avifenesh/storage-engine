# Learning Progress — Columnar Database Engine

## Project Overview
Building a traditional, columnar database to learn storage, execution, SQL layers, and performance engineering on ARM64 platforms. Kernel integration is optional and strictly profile‑gated.

**Focused Goal**: Master storage, vectorized execution, simple SQL, and performance engineering through a pragmatic, userspace‑first project.

## 🎯 Project Scope
- Storage primitives (hash, B+ tree), page/buffer manager, WAL
- Columnar layout with encodings and zone maps; vectorized execution
- SQL subset: parser → optimizer → executor; EXPLAIN
- Performance: SIMD, NUMA/hugepages, io_uring; p50/p99 reporting

## ⚠️ Safety Notice
Kernel work is optional; when used, test only in a VM and keep modules small.

## 📊 Sprint Progress Summary

| Sprint | Focus | Issues | Status | Details |
|--------|-------|--------|--------|---------|
| **Sprint 1** | C Core & Memory | #1-6 | ✅ **COMPLETE** (6/6) | Solid foundation established |
| **Sprint 2** | Hash Storage Engine | #27-30 | 📋 Ready | Userspace baseline, concurrency, metrics |
| **Sprint 3** | B+ Tree Storage | #31-34 | 📋 Planned | Userspace with iterators & ranges |
| **Sprint 4** | Page/Buffer Manager | #? | 📋 Planned | Page layout, buffer & eviction |
| **Sprint 5** | WAL & Recovery | #? | 📋 Planned | WAL, checkpoints, crash replay |

## 🎉 Current Status
**Sprint 1 COMPLETE!** All 6 foundational issues successfully implemented ✅

**Ready for Sprint 2**: Hash Table (Userspace) Baseline

## ✅ Completed Work (Sprint 1)
1. **Issue #1**: Pointer operations (`print_address.c`) ✅
2. **Issue #2**: Dynamic arrays (`dynamic_array.c`) ✅ 
3. **Issue #3**: Memory alignment and struct padding ✅
4. **Issue #4**: 128-bit aligned vectors (`128_aligned_vec.c`) ✅
5. **Issue #5**: Function pointers and callback dispatcher (`vec_dispatcher.c`) ✅
6. **Issue #6**: Makefile with GDB integration and comprehensive build system ✅

## 🚀 Next Steps (Sprint 2)
1. **Issue #27**: Implement userspace hash table with resize and stats
2. **Issue #28**: Add concurrency (striping) and metrics export
3. **Issue #29**: Prepare SIMD plan for hashing/compares (design)
4. **Issue #30**: Build tests and property harness; ASan/Valgrind clean

## 📚 Key Learning Areas

### Sprint 2: Hash Table (Starting Now)
- Linear probing vs chaining tradeoffs; resize policies; probe stats
- Concurrency striping; memory overhead and load factors

### Sprint 3: B+ Tree (Upcoming)
- Node layout, splitting/merging; iterators; range scans

### Later: Vectorized Execution & SQL
- Columnar encodings; vectorized predicates; parser, optimizer, executor

## 📈 Progress Metrics

### Completed
- ✅ 6/6 Sprint 1 issues
- ✅ Solid C foundation
- ✅ Build system ready
- ✅ Development environment configured

### In Progress
- 🔄 Starting Sprint 2: Hash (userspace) work

### Upcoming Milestones
- [ ] Hash baseline and metrics (Sprint 2)
- [ ] B+ tree with iterators (Sprint 3)
- [ ] Page/buffer manager (Sprint 4)
- [ ] WAL + recovery (Sprint 5)

## 🎯 Success Criteria

### Technical Goals
- Publish p50/p99 for core ops
- Meet hash and B+ targets; add WAL durability

### Learning Goals
- Understand storage/execution internals
- Apply SIMD/NUMA/io_uring when they add value

## 📚 Resources
- **Main Kanban**: [STORAGE-ENGINE-KANBAN.md](STORAGE-ENGINE-KANBAN.md)
- **Learning Resources**: [STORAGE_ENGINE_RESOURCES.md](STORAGE_ENGINE_RESOURCES.md)
- **Performance Targets**: [PERFORMANCE_TARGETS.md](PERFORMANCE_TARGETS.md)

---

*Last Updated: Nov 2025 — Columnar DB roadmap aligned*
