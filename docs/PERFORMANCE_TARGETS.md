# 🎯 Performance Targets — Columnar Database

## Overview
Realistic, measurable targets for a userspace columnar database built iteratively. Focus on correctness first; measure p50/p99 and publish simple, repeatable results.

## Core Metrics

### Hash Engine (userspace)
| Metric | Target | Notes |
|--------|--------|-------|
| Put ops/s | ≥ 3k (1 thread), ≥ 15k (8 threads) | ~0.7 load factor |
| Get ops/s | ≥ 10k (1 thread), ≥ 50k (8 threads) | uniform keys |
| Memory overhead | ≤ 1.5× payload | at ~0.7 load |
| p99 probe length | ≤ 8 | linear probing |

### B+ Tree (userspace)
| Metric | Target | Notes |
|--------|--------|-------|
| Lookup ops/s | publish p50/p99 | balanced height |
| Range scan throughput | publish p50/p99 | leaf traversal |
| Node occupancy | ≥ 60% average | steady‑state |

### Page/Buffer + WAL
| Metric | Target | Notes |
|--------|--------|-------|
| Cache hit/miss | publish and compare | controlled workloads |
| Flush latency | bounded; published | fsync grouping |
| Recovery | passes crash‑injection | reproducible harness |

## Sprint‑Specific Targets

### Sprint 2: Hash (Userspace)
- Property tests (100k ops); p99 probe length ≤ 8 at ~0.7 load
- Publish get/put ops/s (1 thread, 8 threads) and memory overhead

### Sprint 3: B+ Tree (Userspace)
- Invariants validated; iterators and range scans; p50/p99 reported

### Later Sprints (Page/Buffer, WAL, Vectorized Exec)
- Cache stats, flush latencies, recovery harness results
- Vectorized scans show ≥ 3× over tuple‑at‑a‑time on simple predicates

## Methodology
- Warm up and measure steady state
- Pin threads to cores when comparing runs
- Record hardware/kernel versions in metrics.json
- Use perf/ftrace/flamegraphs for hotspots and ASan/Valgrind for memory

## Gates
### Must Have
- ✅ Correctness via tests/property tests
- ✅ Memory‑safe (ASan/Valgrind clean)
- ✅ Published p50/p99 and environment notes

### Should Have
- ✅ SIMD speedups when enabled
- ✅ Scaling with thread count (contention understood)
- ✅ Clear performance documentation

### Notes
Hash table lookups are O(1) on average; vectorized scans reduce per‑tuple overhead; WAL with fsync grouping can bound flush latency. Focus on clarity, measurement, and incremental improvement.

