# Sprint 5: Durability & On-Disk Layout

## Overview
Focus on core database durability and persistence. Design an on-disk layout, implement a write-ahead log (WAL), a page cache, crash recovery, and integrity checks. Keep the stack in C to deepen systems knowledge.

**Duration**: 2–3 weeks  
**Prerequisites**: Completed Sprints 2–4 (hash/B+ tree, kernel device, SIMD)

---

## 🎯 Learning Objectives

- Design on-disk page formats (fixed-size pages, headers, checksums)
- Implement WAL with fsync discipline and recovery
- Build an in-process page cache with eviction
- Ensure crash consistency (write ordering, barriers, verification tools)
- Profile I/O modes (buffered vs direct I/O, mmap vs read/write)

---

## 📋 Sprint Issues

### Issue #38: Page Format and On-Disk Layout
**File**: `sprint-5-production-features/disk_format.h`

Design a 4KB page format with a small header and checksum.

Requirements:
- Page header: `page_id`, `page_type`, `lsn` (log sequence number), `checksum`
- Endianness-safe serialization helpers
- CRC32C checksum implementation

Deliverables:
- Header with structs and helpers
- Tests that serialize/deserialize pages and verify checksum

---

### Issue #39: Write-Ahead Log (WAL)
**Files**: `sprint-5-production-features/wal.c`, `sprint-5-production-features/wal.h`

Implement an append-only WAL with fsync and recovery.

Requirements:
- Log record header with length, `lsn`, checksum
- Append, flush, and rotate files
- Crash-safe recovery that replays committed records, ignores torn/partial

Deliverables:
- WAL API with `wal_open/close/append/flush/replay`
- Fsync tests and crash-replay simulation (truncate mid-record)

---

### Issue #40: Page Cache with Eviction
**Files**: `sprint-5-production-features/page_cache.c`, `sprint-5-production-features/page_cache.h`

Implement an in-process page cache with LRU or clock eviction.

Requirements:
- Hash table for page lookup
- Pin/unpin API and dirty bit with writeback policy
- Background flush thread (optional)

Deliverables:
- Cache API: `pc_get`, `pc_pin/unpin`, `pc_mark_dirty`, `pc_evict`
- Stress tests with random read/write workload

---

### Issue #41: Recovery and Verification Tools
**Files**: `sprint-5-production-features/recovery.c`, `sprint-5-production-features/fsck.c`

Build startup recovery and an offline verifier.

Requirements:
- Startup recovery: scan WAL, rebuild dirty pages, persist
- `fsck` tool: verify checksums, page headers, and LSN monotonicity

Deliverables:
- Recovery routine with tests that inject crashes at different points
- Simple `fsck` binary that prints findings

---

### Issue #42: I/O Profiling and Modes
**File**: `sprint-5-production-features/io_profile.c`

Measure buffered vs `O_DIRECT`, `read/write` vs `mmap`, and batching effects.

Deliverables:
- Benchmark program that toggles I/O modes and prints throughput/latency

---

### Issue #43: Add Comprehensive Testing and Error Handling
**File**: `sprint-5-production-features/test_suite.c`

Requirements:
- Unit tests for disk format, WAL, cache
- Crash-injection tests for recovery
- Stress testing for concurrent page access
- Memory leak detection

Test Framework Sketch:
```c
// Minimal skeleton for test cases; expand as needed
typedef struct { const char *name; void (*fn)(void); } test_case_t;
void run_tests(test_case_t *cases, size_t n);
```

Success Criteria:
- All unit tests pass
- Crash-injection tests pass
- No crashes under stress testing
- Valgrind reports no memory leaks

---

## 🛠️ Implementation Guide

### Week 1: Disk Format + WAL (userspace)
1. Implement page structs and CRC32C
2. Implement WAL append/flush and replay
3. Unit test serialization and recovery

### Week 2: Page Cache
1. Implement lookup and pinning
2. Implement eviction and writeback
3. Stress test with random workloads

### Week 3: Integration and Profiling
1. Wire WAL + cache to your hash/B+ tree
2. Measure throughput with/without cache
3. Tune fsync batching and thresholds

### Week 4: Tools and Docs
1. Implement fsck-style verifier
2. Add crash-injection tests
3. Write the performance guide

---

## 📊 Testing Strategy

### Load and Performance Testing
```bash
# Storage engine microbenchmarks
./storage_benchmark --workload=write_heavy --duration=300
./storage_benchmark --workload=read_heavy --duration=300
./storage_benchmark --workload=mixed --duration=300
```

### Profiling Tools
```bash
# CPU profiling
perf record -g ./benchmark
perf report

# Heap profiling
valgrind --tool=massif ./engine
ms_print massif.out.*

# Cache analysis
valgrind --tool=cachegrind ./benchmark
cg_annotate cachegrind.out.*
```

---

## 📚 Resources

- SQLite Design docs (B-tree + WAL)
- ARIES/IM papers (WAL and recovery)
- Linux VFS and storage stack documentation

---

## ✅ Deliverables

By the end of Sprint 5, you should have:

1. On-disk page format with checksums
2. Write-ahead log with flush/replay
3. Page cache with eviction and writeback
4. Recovery routine that passes crash-injection tests
5. Performance guide with I/O profiling data

---

## 🎉 Project Track Complete!

You’ve built a storage engine core with:
- Multiple in-memory engines (Hash, B+ Tree)
- Linux kernel integration via character devices
- SIMD-optimized hot paths on ARM
- Durable on-disk layout with WAL and recovery

### Next Steps
- Add compaction + SSTables (LSM tree)
- Implement background checkpoints
- Explore zero-copy I/O (io_uring, O_DIRECT)
- Study ARIES details and partial-page logging

---

## ✅ Quality Gates (for your implementation)

- Unit tests: page serialization + checksum, WAL append/flush/replay
- Crash‑injection: replay resilience for truncated/torn records
- Page cache: eviction logic under pinning, flush callback used correctly
- Sanitizers: run with ASan/Valgrind to check for leaks and UBs
- Performance notes: record fsync latency and cache hit ratio on your machine

Provided scaffolding helps you test; solutions are intentionally not included.

---

## 🚀 Expert Track (Optional, Fast Path)

- WAL: group commit, fdatasync batching, torn‑write detection, idempotent replay, LSN epochs.
- Page cache: clock‑pro or 2Q, dirty clustering for writeback, pinning under pressure.
- I/O: compare buffered vs O_DIRECT vs mmap; io_uring (optional); quantify fsync costs.
- QA: crash matrix checkpoints, page LSN monotonicity, fsck noise‑free on clean shutdowns.

---

## 📏 Quality Gates & Targets

- WAL & Recovery
  - Replay a 1 GB log in ≤ 3 s (sequential I/O) on Pi 5; ≤ 6 s on Pi 4
  - Crash matrix: inject at ≥ 10 checkpoints; 100% idempotent replays; no double-apply
  - Group commit: batching improves fsync throughput ≥ 2x over per-op fsync at 1 KB records
- Page Cache
  - Hit ratio ≥ 90% on 80/20 read-heavy workload at steady state
  - Eviction: never evict pinned pages; dirty writeback latency ≤ 1 s (configurable)
- Integrity
  - fsck: 0 errors after clean shutdown; detects checksum and LSN monotonicity violations
  - Page header CRC32C validates payload; corrupted pages are flagged and skipped
- I/O Profiling
  - Report p50/p99 fsync latency; compare buffered vs O_DIRECT vs mmap
  - Document trade-offs observed on your hardware (2–3 bullets)

---

## 🧪 Measurement Checklist

- Run Sprint 5 tests/tools: `make test-sprint5` (builds `s5_test`, `s5_recovery`, `s5_fsck`, `s5_ioprof`)
- WAL: time `s5_recovery` on a large log; run `chaos_replay` (via `make test-expert`) to validate partial‑record handling
- Page cache: instrument hit/miss/eviction/dirty stats in your cache; record steady‑state hit ratio
- I/O: run `build/s5_ioprof <file> <bytes>`; capture fsync p50/p99 and note buffered vs O_DIRECT vs mmap
- Fill `metrics.json.sprint5_durability` with measured values and notes
