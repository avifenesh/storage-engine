# Sprint 12: Concurrency and NUMA

Goal: Sharded structures, per‑CPU stats, core pinning, and hugepages/NUMA‑aware allocation.

Acceptance:
- Scaling measurements across threads and sockets; reduced cross‑socket traffic.
- Configurable shard/stripe counts; contention analysis presented.

## Learning Resources

- Articles on NUMA architectures and memory locality (including Linux NUMA documentation).
- Blog posts or papers on sharded or partitioned data structures.
- Tutorials on thread pinning, CPU affinity, and hugepages configuration.

## Core Questions

- Which structures in your engine are the best candidates for sharding (buffer pool, hash tables, lock managers, statistics)?
- How will you map shards to cores or NUMA nodes?
- How will you measure cross‑socket traffic and remote memory access?
- What strategies will you use for per‑thread or per‑shard statistics and aggregation?

## Experiments & Measurements

- Run the same workload with and without sharding and compare throughput and tail latency.
- Use tools such as `numactl`, `perf`, and hardware counters to observe NUMA effects and cache coherence traffic.
- Experiment with different shard counts, thread affinity settings, and page sizes (hugepages vs normal) and record the impact.
