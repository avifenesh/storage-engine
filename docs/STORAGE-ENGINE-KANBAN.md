# 📋 Row-Store Database Engine – Project Kanban

## 🎯 Project Vision
Build a learning‑first row‑store database engine (heap tables plus B+ tree/hash indexes) to master storage, execution, SQL, concurrency, and performance engineering on ARM64. Kernel integration is optional and profile‑gated; columnar projections and vectorized execution are reach goals.

**Realistic Goals**:
- Userspace row‑store engine with heap tables and hash/B+ tree indexes
- Solid iterator‑based execution over heap and indexes; optional vectorized paths later
- WAL durability with crash recovery
- Meaningful p50/p99 metrics and repeatable benchmarks

## 🧭 Recommended Learning Order

1. Foundations: Sprint 1 (C core & memory fundamentals).
2. Core data structures: Sprint 2 (hash table) and Sprint 3 (B+ tree).
3. Storage spine (row store): Sprint 4 (page format, slotted pages, buffer manager) and Sprint 5 (WAL & recovery).
4. SQL & execution: Sprint 6 (SQL parser/AST), Sprint 7 (logical optimizer), Sprint 8 (cost model), Sprint 9 (joins, aggregation, sort).
5. Transactions & concurrency: Sprint 10 (MVCC), Sprint 11 (locking & latching), Sprint 12 (concurrency & NUMA).
6. Acceleration & reach: Sprint 13 (columnar layout), Sprint 14 (vectorized executor), Sprint 15 (SIMD & compression), Sprint 16 (io_uring I/O), Sprint 17 (observability & benchmarks).

---

## 📅 Sprint 1: C Core & Memory Fundamentals ✅ **SPRINT COMPLETE**

### ✅ Completed (Keep as Reference)
- **Issue #1** - Review pointer operations in C ✅
- **Issue #2** - Build dynamic array (malloc, realloc, free) ✅
- **Issue #3** - Study memory alignment and struct padding ✅
- **Issue #4** - Build aligned struct for 128-bit vectors ✅
- **Issue #5** - Learn function pointers for callback dispatchers ✅
- **Issue #6** - Setup Makefile with proper flags and GDB integration ✅

---

## 📅 Sprint 2: Hash Storage Engine (2–3 weeks)

### Learning Objectives
- Implement hash table with dynamic resize and probe stats
- Add concurrency striping and per‑stripe metrics
- Establish baseline microbenchmarks and metrics export

### 📋 Issues
- **Issue #27** - Implement core hash table engine 📋 **READY**
  - Userspace baseline, dynamic resize, probe stats
  - Concurrency striping and per‑stripe metrics
  - Resources: [Hash Tables in C](https://benhoyt.com/writings/hash-table-in-c/)

- **Issue #28** - Export metrics and benchmarks 📋 **READY**
  - Record ops/s, p50/p99, probe histogram
  - Export to metrics.json; compare variants

- **Issue #29** - Prepare SIMD plan 📋 **READY**
  - Identify hot loops (hashing, compares)
  - Plan runtime dispatch and tests

- **Issue #30** - Tests and memory safety 📋 **READY**
  - Unit/property tests; concurrent smoke tests
  - ASan/Valgrind clean

---

## 📅 Sprint 3: B+ Tree Storage Engine (3–4 weeks)

### Learning Objectives
- Master B+ tree algorithms and balanced tree operations
- Implement efficient range queries and key iteration
- Optimize for storage with disk-friendly node layouts
- Handle concurrent access with multi-reader single-writer patterns

### 📋 Issues
- Core B+ tree data structure 📋 **BACKLOG**
  - Userspace implementation, configurable node size/order
  - Iterators and range scans; invariants validated
  - Benchmarks for point/range workloads

---

## 📅 Sprint 4: Page/Buffer Manager (2–3 weeks)
Design page layout and build a buffer manager with eviction, pinning, and prefetch. Benchmarks for cache hit/miss and latency.

---

## 📅 Sprint 5: WAL and Recovery (3–4 weeks)
Implement WAL, checkpoints, and crash recovery. Profile I/O (io_uring optional).

### Learning Objectives
- Implement WAL append/flush/replay with checksums
- Add checkpoints and recovery; verify crash safety
- Profile I/O and batching (io_uring optional)

### 📋 Issues
- WAL format + replay + checkpoints 📋 **BACKLOG**
- Page cache integration and writeback policy 📋 **BACKLOG**

---

## 📊 Progress Summary

| Sprint | Focus | Issues | Status | Timeline |
|--------|-------|--------|--------|----------|
| Sprint 1 | C Fundamentals | #1-6 | ✅ COMPLETE | Done |
| Sprint 2 | Hash (userspace) | #27-30 | 📋 Ready | 2–3 weeks |
| Sprint 3 | B+ tree (userspace) | #31-34 | 📋 Backlog | 3–4 weeks |
| Sprint 4 | Page/buffer | #? | 📋 Backlog | 2–3 weeks |
| Sprint 5 | WAL/recovery | #? | 📋 Backlog | 3–4 weeks |

## 🎯 Success Metrics

### Performance Targets
- Hash: ≥ 10k ops/s single‑thread; ≥ 50k ops/s at 8 threads; p99 under control
- B+ tree: efficient range scans; p50 and p99 measured; balanced height
- Vectorized scan filters: 3× over tuple‑at‑a‑time on simple predicates
- WAL: bounded flush latency; recover cleanly from injected crashes

### Learning Outcomes
- ✅ Solid understanding of storage, execution, and concurrency internals
- ✅ Ability to measure and interpret p50/p99 metrics
- ✅ Familiarity with SIMD and I/O techniques where they add value

---

*Last Updated: Nov 2025 – Row‑store engine roadmap aligned*
