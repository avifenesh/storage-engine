# Sprint 8: Joins, Aggregation, and Sort

Goal: Hash join (build/probe, spill), group‑by (hash/sort), and external sort.

Acceptance:
- Correctness on mixed workloads; spill logic documented where applicable.
- Benchmarks for join, group, and sort with p50/p99 and memory use.
- Plan/operator notes integrated with vectorized executor.

