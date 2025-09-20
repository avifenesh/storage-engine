Sprint 3: B+ Tree Storage Engine

This directory holds a minimal userspace prototype and stubs you can expand while following docs/sprints/sprint-3-btree-storage.md.

Suggested order:
- Implement userspace B+ tree in `btree_engine.c`
- Write microbenchmarks in `btree_benchmarks.c`
- Explore SIMD key compare in `neon_compare.c`
- Mirror APIs into kernel code later (separate module/headers)

