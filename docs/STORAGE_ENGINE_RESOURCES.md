# 📚 Storage Engine Development — Curated Resources

Concise, high‑value references for a userspace columnar database project. Focus on storage, vectorized execution, and durability. Kernel topics are optional and profile‑gated.

## Core Systems & Algorithms
- Database Internals (J. Petrov) — storage, indexing, transactions
- CMU 15‑445/645 Database Systems — lectures and notes
- Hash tables in C (Ben Hoyt) — practical design tradeoffs
- SQLite file format and architecture — B+ tree and WAL

## Columnar & Vectorized Execution
- MonetDB/X100 and vectorized execution papers (scan, filter, project)
- PAX, Parquet, and Apache Arrow concepts (column groups, encodings)
- SIMD selection and predicate evaluation techniques

## Durability & Buffering
- ARIES (WAL and recovery) overview
- Page formats, buffer management, replacement policies

## Performance & Tooling
- Linux perf tools (perf/ftrace/flamegraphs)
- NUMA/hugepages basics and memory locality

## Implementation References
- SQLite — embedded engine; B+ tree and WAL
- LMDB — memory‑mapped B+ tree (design study)

Use these to guide design and measurement; keep scope tight and code minimal.

