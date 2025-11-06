# Sprint 3: B+ Tree Storage Engine (Userspace)

## Overview
Design and implement a userspace B+ tree with efficient range scans and stable iterators. Build a correctness baseline and microbenchmarks. Kernel work is out of scope for this sprint.

**Duration**: 3–4 weeks  
**Prerequisites**: Sprint 2 (hash table baseline)

---

## Learning Objectives
- Implement insertion, lookup, deletion; maintain invariants and height balance.
- Provide range scans and iterators over leaves with predictable performance.
- Choose node/page size with cache/page considerations; prepare for page manager.
- Measure throughput/latency for point/range workloads; document findings.

---

## Sprint Issues
- Core B+ Tree (userspace): node layout and invariants; splitting/merging; leaf linkage for ranges; configurable order; export stats (height, nodes, occupancy).
- Range Scans and Iterators: forward range scans via leaf traversal; iterator semantics under modification.
- Benchmarks and Property Tests: randomized insert/search/delete vs an oracle; timed runs for point/range workloads; report p50/p99, memory overhead, occupancy distribution.

---

## Acceptance Criteria
- Correctness: property tests pass (100k ops); invariants verified after ops.
- Performance: publish p50/p99 for point/range workloads; memory overhead reported.
- Documentation: design notes, tradeoffs, measurement results, retrospective.

---

## Out of Scope (Next Sprints)
- Kernel devices/IOCTL.
- Persistence/page manager integration.
- SIMD‑accelerated comparisons (planned later).

