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
- Persistence/page manager integration.
- SIMD‑accelerated comparisons (planned later).

## Learning Resources

- CMU 15‑445 lectures and notes on B+ trees and indexing.
- SQLite documentation and code comments describing its B‑tree pages and operations.
- "Database Internals" (J. Petrov) chapters on indexing and B+ trees.
- CLRS (Cormen et al.) sections on B‑trees for conceptual grounding.

## Core Questions

- How does a B+ tree differ from a plain B‑tree, and why is that better for range scans and disk‑friendly access?
- How do node size and fan‑out affect tree height, cache behavior, and I/O efficiency?
- What invariants must hold for internal and leaf nodes after every insert and delete?
- How will you represent keys (fixed‑width vs variable‑length) and support composite keys?
- How should iterators behave when concurrent modifications occur, and what semantics are acceptable for this project?

## Experiments & Measurements

- Benchmark random inserts and lookups for different node sizes (e.g., 512 B, 1 KiB, 4 KiB) and record tree height and throughput.
- Compare B+ tree point lookup and range scan performance to your hash table for various key distributions.
- Implement invariants checks that walk the whole tree after random operations and validate ordering, occupancy, and leaf linkage.
- Measure how sequential vs random key insertion patterns affect tree shape and performance.
