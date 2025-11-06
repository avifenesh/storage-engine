# 📚 Columnar Database Engine — Documentation

## Project Overview

This is a self‑learning project to build a traditional, columnar database with a pragmatic SQL subset (CREATE/INSERT/SELECT/JOIN/GROUP/ORDER), developed incrementally from core storage primitives to a vectorized execution engine. Kernel integration is optional and strictly profile‑gated.

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
- Foundations:
  - **[sprint-1-c-fundamentals.md](sprints/sprint-1-c-fundamentals.md)** ✅ Completed
  - **[sprint-2-hash-storage.md](sprints/sprint-2-hash-storage.md)** – Hash table (userspace) baseline and measurements
  - **[sprint-3-btree-storage.md](sprints/sprint-3-btree-storage.md)** – B+ tree (userspace) with iterators
- Storage Engine:
  - **sprint-4-page-buffer.md** – Page format and buffer manager
  - **sprint-5-wal-recovery.md** – WAL, checkpoints, crash recovery
  - **sprint-6-columnar-layout.md** – Column groups, zone maps, encoding
- Execution & SQL:
  - **sprint-7-vectorized-executor.md** – Data chunks, filters, projection
  - **sprint-8-joins-agg-sort.md** – Hash join, group-by, external sort
  - **sprint-9-sql-parser.md** – Grammar, AST, EXPLAIN skeleton
  - **sprint-10-logical-optimizer.md** – Algebra, rule-based rewrite
  - **sprint-11-cost-model.md** – Stats, cardinality, cost-driven plans
- Concurrency & Performance:
  - **sprint-12-mvcc.md** – MVCC and vacuum
  - **sprint-13-locking.md** – Lock manager and latching
  - **sprint-14-concurrency-numa.md** – Sharding, NUMA/hugepages
  - **sprint-15-simd-compression.md** – SIMD filters and codecs
  - **sprint-16-io_uring.md** – Async I/O pipeline and prefetch
  - **sprint-17-observability.md** – Benchmarks, metrics, regressions

### Additional Resources
- See root `README.md` for Make targets reference

## 🎯 Project Goals

### Realistic Scope
- Target: A functional analytical (columnar) DB with a pragmatic SQL subset
- Performance: Vectorized execution with competitive p50/p99; WAL durability
- Learning: 3–6 months; focus on storage, execution, and optimization

### What We're Building
1. Userspace storage: hash + B+ tree; page/buffer manager; WAL
2. Columnar layout: encoding, zone maps, vectorized scans
3. SQL layer: parser → logical optimizer → executor
4. Performance: SIMD paths, NUMA/hugepages, io_uring I/O
5. Optional kernel work: eBPF/XDP/targeted modules if justified

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
