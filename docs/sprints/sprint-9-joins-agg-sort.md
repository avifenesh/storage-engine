# Sprint 9: Joins, Aggregation, and Sort

Goal: Hash join (build/probe, spill), group‑by (hash/sort), and external sort.

Acceptance:
- Correctness on mixed workloads; spill logic documented where applicable.
- Benchmarks for join, group, and sort with p50/p99 and memory use.
- Plan/operator notes integrated with vectorized executor.

## Learning Resources

- Database textbooks and CMU 15‑445 lectures on joins, aggregation, and sorting.
- Papers and blog posts on hash join, grace hash join, and external merge sort.
- Documentation or posts from systems like PostgreSQL and SQLite describing their join and sort strategies.

## Core Questions

- When should you choose hash join vs nested‑loop vs sort‑merge for a given workload?
- How will you structure hash tables for joins (build/probe) and handle spilling to disk?
- What grouping strategies will you support (hash‑based vs sort‑based), and when is each preferable?
- How will you design your sort operator to handle data that does not fit in memory?
- How will join, group‑by, and sort operators integrate with both row‑store and vectorized execution paths?

## Experiments & Measurements

- Benchmark join performance for different join orders and sizes (small vs large build side) and compare algorithms.
- Measure group‑by performance under different key distributions (uniform, skewed, many distinct vs few).
- Test external sort on data sets that exceed memory, measuring I/O volume, number of passes, and runtime.
- Use microbenchmarks to compare plans chosen by simple heuristics vs forced alternate strategies for the same queries.
