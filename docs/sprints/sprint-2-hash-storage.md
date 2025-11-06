# Sprint 2: Hash Table Storage Engine (Userspace)

## Overview
Build a correct, measurable userspace hash table with linear probing (or chaining), dynamic resizing, and basic concurrency. Establish a throughput/latency baseline and probe statistics to inform later design. Kernel integration is optional and not part of this sprint.

**Duration**: 2–3 weeks  
**Prerequisites**: Sprint 1 (C fundamentals)

---

## 🎯 Learning Objectives

1. Implement collision resolution and dynamic resizing (measure load factor and probe length)
2. Add basic concurrency (global lock → striped locks) and per‑stripe stats
3. Produce a baseline of p50/p99 latency and ops/s; export probe-length histograms
4. Prepare for future SIMD hashing/comparison by separating hot loops

Tip: Start with a simple, correct userspace implementation, then port to kernel space. Add SIMD and concurrency last.

---

## 📋 Sprint Issues

### Issue #27: Implement Hash Table with Collision Resolution (Userspace)

**Requirements**:
- Implement either linear probing or chaining (pick one to start)
- Dynamic resizing around ~0.7–0.75 load factor
- Counters for items and approximate memory
- Userspace allocation; kernel is out of scope here

Note: Keep public headers under `include/storage/` and implementation in `src/storage/hash/`. Avoid kernel specifics here.

**Success Criteria**:
- Property tests pass (100k random ops against oracle)
- Probe p99 ≤ 8 at ~0.7 load (linear probing); or average chain ≤ 1.5 (chaining)
- Memory overhead ≤ 150% of raw payload at target load

Lab Steps:
- Step 1: Implement userspace hash table (no locks). Add tests in `tests/`.
- Step 2: Add resizing; export load factor and probe histogram.
- Step 3: Add concurrency: global lock → striped locks; measure scaling.
- Step 4: Write a short retrospective: findings, bottlenecks, next steps.

---

### Issue #28: Export Hash Metrics and Benchmarks
Produce a simple benchmark harness that records ops/s, p50/p99, and probe stats (or chain length). Export to `metrics.json`.

---

### Issue #29: Prepare for SIMD Paths (Design)
Identify hot loops (hashing, compares). Document a plan for runtime dispatch to SIMD vs scalar without changing public APIs.

---

### Issue #30: Tests and Memory Safety
Unit/property tests for put/get/delete under random workloads; multi‑thread smoke tests; ASan/Valgrind clean.

Targets (Raspberry Pi 4/5; scale relatively):
- Reads: ≥ 10k ops/s (single‑thread), ≥ 50k ops/s (8 threads)
- Writes: ≥ 3k ops/s (single‑thread), ≥ 15k ops/s (8 threads)
- Memory overhead: ≤ 150% of raw payload at ~0.7 load

---

## 🛠️ Implementation Guide

Week 1: Core engine and resize; initial tests and probe stats  
Week 2: Concurrency (striping), metrics export, benchmarks, retrospective

Remove stretch topics from this sprint; SIMD and further optimization are in later sprints.

Learning Checklist:
- Can explain trade-offs of linear probing vs chaining
- Can show atomics and locking used correctly in kernel code
- Can demonstrate measurable speedups from NEON

---

## 🔒 What’s Provided vs What You Build

- Provided:
  - Build system, skeleton headers and layout (`include/`, `src/`, `tests/`)
  - Example microbench target under `bench/`
- You Build:
  - Hash table algorithm (probing/chaining), resizing, stats
  - Correctness under concurrency (striped locks)
  - Benchmarks and measurements

No solutions are provided; only QA scaffolding to validate your own implementation.

---

## 🚀 Expert Track (Optional, Fast Path)

- Probing: Robin Hood hashing with tombstones; bounded probe length histograms.
- Resizing: incremental, lock‑striped or RCU‑based without stop‑the‑world.
- Concurrency: per‑bucket spinlocks or lock striping; try read‑mostly lock‑free gets.
- Hashing: feature‑gated CRC32/NEON with runtime dispatch; alignment + tail handling.
- QA: property tests vs an oracle map; contention profiling; p50/p99 under 8 threads.

---

## 📏 Quality Gates & Targets

Use these as measurable finish lines for this sprint. Adjust for your hardware, but keep the ratios.

- Correctness
  - Property tests: 100k random ops (mix of put/get/delete) match oracle map
  - Concurrency: 8 threads, 1M total ops without corruption or deadlocks
  - Load factor stays within [0.2, 0.75] with automatic resize
- Hash quality
  - Uniform random keys: bucket occupancy stddev < 15% of mean
  - Probe length (open addressing): p99 ≤ 8, max ≤ 32
  - Collision rate (chaining): average chain length ≤ 1.5 at 0.75 load factor
- Performance (Raspberry Pi 4/5 baseline; scale relatively on other HW)
  - Reads: ≥ 10k ops/s single-thread; ≥ 50k ops/s at 8 threads
  - Writes: ≥ 3k ops/s single-thread; ≥ 15k ops/s at 8 threads
  - NEON/CRC32 path: ≥ 2.5x speedup vs scalar hashing on ≥16B keys
- Memory
  - Overhead ≤ 150% of raw KV footprint at 0.6–0.75 load factor

---

## 🧪 Measurement Checklist

- Build/run expert harness: `make test-expert` (runs `build/hash_prop`)
- Throughput: add a simple timed loop in your hash tests; record read/write ops/s (1 thread, then 8 threads)
- Hash quality: export bucket occupancy/probe length stats; fill `metrics.json` fields under `sprint2_hash`
- Record environment: CPU model, core count, kernel version in a comment near the metrics

---

## 📊 Testing Strategy

### Unit Tests
Keep unit/property tests small and deterministic; put binaries under `tests/`.

### Performance Tests
Use `make bench` or `make run-bench` to build and run local microbenchmarks.

### SIMD Verification
Plan only; actual SIMD work is deferred to later sprints.

---

## 🐛 Common Issues and Solutions

### Issue: Hash table performance degrades
**Solution**: Monitor load factor, implement dynamic resizing, tune hash function

### Issue: High collision rate
**Solution**: Switch to a stronger hash, lower load factor, or adopt robin‑hood probing.

---

## 📚 Resources

### Hash Table Theory
- [Hash Tables Explained](https://benhoyt.com/writings/hash-table-in-c/)
- [Collision Resolution Strategies](https://en.wikipedia.org/wiki/Hash_table#Collision_resolution)
- [Performance Analysis](https://github.com/attractivechaos/klib)

### Optional (Later) SIMD Reading
- [ARM NEON Programmer's Guide](https://developer.arm.com/documentation/102474/latest/)
- [NEON Intrinsics Reference](https://arm-software.github.io/acle/neon_intrinsics/advsimd.html)
- [SIMD Hash Functions](https://lemire.me/blog/2021/01/29/arm-neon-programming-quick-reference/)

### ARM NEON Optimization
- [ARM NEON Programmer's Guide](https://developer.arm.com/documentation/102474/latest/)
- [NEON Intrinsics Reference](https://arm-software.github.io/acle/neon_intrinsics/advsimd.html)
- [SIMD Hash Functions](https://lemire.me/blog/2021/01/29/arm-neon-programming-quick-reference/)

---

## ✅ Deliverables

By the end of Sprint 2, you should have:

1. Working userspace hash engine with collision resolution
2. Comprehensive tests with property checks and microbenchmarks
3. Memory‑safe implementation (ASan/Valgrind clean)
4. Documentation with design notes and measurements

---

## 🚀 Next Sprint Preview

Sprint 3 will build a userspace B+ tree storage engine with range scans and iterators.

Your hash table foundation will inform the design patterns for more complex storage engines!

---

*Remember: Hash tables are the foundation of many storage systems. Master them well before moving to more complex data structures.*
