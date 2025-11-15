# 📚 Row-Store Database Engine — Documentation

## Project Overview

This is a self‑learning project to build a traditional row‑store database engine (heap tables plus B+ tree/hash indexes) with a pragmatic SQL subset (CREATE/INSERT/SELECT/JOIN/GROUP/ORDER), developed incrementally from storage primitives (heap files, indexes, pages, WAL) to a full engine. Columnar projections and vectorized execution are optional reach goals. Kernel integration is optional and strictly profile‑gated.

Guiding principles:
- Userspace first (sharding, NUMA, hugepages, SIMD, io_uring); add kernel features only when profiling proves a benefit and safe fallbacks exist.
- Sprints are documentation and measurement guides; solutions are not provided.
- Performance is measured locally, not in CI.

## 📂 Documentation Structure

### Core Project Documents
- **[STORAGE-ENGINE-KANBAN.md](STORAGE-ENGINE-KANBAN.md)** - Roadmap and sprint tracking
- **[LEARNING_PROGRESS.md](LEARNING_PROGRESS.md)** - Track your learning journey
- **[STORAGE_ENGINE_RESOURCES.md](STORAGE_ENGINE_RESOURCES.md)** - Curated learning resources
- **[PERFORMANCE_TARGETS.md](PERFORMANCE_TARGETS.md)** - Benchmarks and goals

### Sprint Documentation
- Recommended learning order (row‑store first, accelerators later), aligned with sprint numbers:
- Foundations:
  - **[sprint-1-c-core-memory.md](sprints/sprint-1-c-core-memory.md)** – C core refresh and memory fundamentals ✅
  - **[sprint-2-hash-storage.md](sprints/sprint-2-hash-storage.md)** – Hash table (userspace) baseline and measurements
  - **[sprint-3-btree-storage.md](sprints/sprint-3-btree-storage.md)** – B+ tree (userspace) with iterators and ranges
- Storage spine (row store):
  - **[sprint-4-page-buffer.md](sprints/sprint-4-page-buffer.md)** – Page format, slotted pages, buffer manager
  - **[sprint-5-wal-recovery.md](sprints/sprint-5-wal-recovery.md)** – WAL, checkpoints, crash recovery
- SQL & execution:
  - **[sprint-6-sql-parser.md](sprints/sprint-6-sql-parser.md)** – Grammar, AST, EXPLAIN skeleton
  - **[sprint-7-logical-optimizer.md](sprints/sprint-7-logical-optimizer.md)** – Algebra and rule‑based rewrite
  - **[sprint-8-cost-model.md](sprints/sprint-8-cost-model.md)** – Stats, cardinality, and cost‑driven plans
  - **[sprint-9-joins-agg-sort.md](sprints/sprint-9-joins-agg-sort.md)** – Hash join, group‑by, external sort
- Transactions & concurrency:
  - **[sprint-10-mvcc.md](sprints/sprint-10-mvcc.md)** – MVCC and vacuum
  - **[sprint-11-locking.md](sprints/sprint-11-locking.md)** – Lock manager and latching
  - **[sprint-12-concurrency-numa.md](sprints/sprint-12-concurrency-numa.md)** – Sharding, NUMA/hugepages
- Acceleration & reach:
  - **[sprint-13-columnar-layout.md](sprints/sprint-13-columnar-layout.md)** – Column groups, zone maps, encodings as optional projections
  - **[sprint-14-vectorized-executor.md](sprints/sprint-14-vectorized-executor.md)** – Data chunks, filters, projection (vectorized executor)
  - **[sprint-15-simd-compression.md](sprints/sprint-15-simd-compression.md)** – SIMD filters and codecs
  - **[sprint-16-io_uring.md](sprints/sprint-16-io_uring.md)** – Async I/O pipeline and prefetch
  - **[sprint-17-observability.md](sprints/sprint-17-observability.md)** – Benchmarks, metrics, regressions

### Additional Resources
- See root `README.md` for Make targets reference

## 🎯 Project Goals

### Realistic Scope
- Target: A functional row‑store database engine with a pragmatic SQL subset
- Performance: Solid heap/index and page/buffer performance with WAL durability; later vectorized execution with competitive p50/p99 where appropriate
- Learning: 3–6 months; focus on storage, execution, concurrency, and optimization

### What We're Building
1. Userspace row storage: heap tables, hash + B+ tree indexes; page/buffer manager; WAL
2. Optional columnar projections: encodings, zone maps, and vectorized scans built on top of the row store
3. SQL layer: parser → logical optimizer → executor
4. Performance: SIMD paths, NUMA/hugepages, io_uring I/O where they add value
5. Optional kernel work: eBPF/XDP/targeted modules if justified and safely profile‑gated

## ⚠️ IMPORTANT SAFETY WARNINGS

### Kernel Development Risks
- **System Crashes**: Kernel bugs can crash your entire system
- **Data Loss**: Kernel panics may corrupt filesystems
- **Security**: Improper validation can create security vulnerabilities
- **Hardware**: Incorrect drivers can damage hardware (rare but possible)

### Safety Precautions
1. **ALWAYS test in a virtual machine first**
2. **Keep backups of important data**
3. **Never test on production systems**
4. **Use a dedicated development machine if possible**
5. **Enable kernel debugging options**

## 🚀 Getting Started

### Prerequisites
- **C Programming**: Solid understanding of pointers, memory management, structs
- **Linux Knowledge**: 
  - Command line proficiency
  - Understanding of file systems and permissions
  - Basic kernel concepts (user/kernel space)
- **Development Environment**:
  - ARM64 Linux system (or VM)
  - GCC 9+ with ARM64 support
  - (Optional) Linux kernel headers if you plan to explore kernel topics
  - Git for version control
  - Make (GNU Make 4.0+)
- **Hardware Requirements**:
  - Minimum 2GB RAM (4GB recommended)
  - 10GB free disk space
  - ARM64 processor (or emulation)

### Recommended Setup
Use a VM for kernel experiments; measure performance locally, not in CI.

### Quick Start
1. Complete Sprint 1 if not already done ✅
2. (Optional) Set up a VM if you plan kernel experiments
3. Read the sprint guides under docs/sprints/ starting with sprint-2-hash-storage.md
4. Follow the implementation guide in each sprint
5. Use the Makefile for building and testing

## 📈 Learning Path (High Level)

Foundations → Storage → Execution → SQL → Concurrency → Performance & Ops (see Sprint Documentation above for details).

## 🔧 Development Tools

- **Build System**: Comprehensive Makefile with educational features
- **Memory Safety**: AddressSanitizer and Valgrind integration
- **Assembly Analysis**: Educational assembly generation
- **Performance Tools**: Built-in benchmarking support

## 📝 Notes

Sprints guide learning and measurements; no code solutions are included. Kernel work is optional and gated by profiling.

---

*Last Updated: Nov 2025 – Columnar DB roadmap and kernel gating*
