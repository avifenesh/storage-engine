# 📚 Storage Engine Development — Curated Resources

Concise, high‑value references for a userspace row‑store database engine project, with optional columnar and vectorized features. Focus on storage, execution, durability, concurrency, and performance. Kernel topics are optional and profile‑gated.

## Core Systems & Algorithms
- Database Internals (J. Petrov) — storage, indexing, transactions
- CMU 15‑445/645 Database Systems — lectures and notes
- Hash tables in C (Ben Hoyt) — practical design tradeoffs
- SQLite file format and architecture — heap/B+ tree and WAL
- "Architecture of a Database System" (Hellerstein et al.) — high‑level DBMS architecture

## Columnar & Vectorized Execution (Optional Accelerators)
- MonetDB/X100 and vectorized execution papers (scan, filter, project)
- PAX, Parquet, and Apache Arrow concepts (column groups, encodings)
- SIMD selection and predicate evaluation techniques

## Durability & Buffering
- ARIES (WAL and recovery) overview
- Page formats, buffer management, replacement policies

## Transactions & Concurrency
- CMU 15‑445 lectures on transactions, locking, and MVCC
- "Database Internals" sections on concurrency control and multiversioning
- Classic papers and summaries on 2PL, MVCC, and isolation levels

## OS & Multiprocessing
- "The Linux Programming Interface" (Kerrisk) — processes, threads, file I/O
- POSIX threads, atomics, and synchronization primitives (man pages and tutorials)
- Linux man pages and guides for `mmap`, `pread`, `pwrite`, `fsync`, and `io_uring`

## Performance & Tooling
- Linux perf tools (perf/ftrace/flamegraphs)
- NUMA/hugepages basics and memory locality

## Implementation References
- SQLite — embedded engine; heap/B+ tree and WAL
- LMDB — memory‑mapped B+ tree (design study)

Use these to guide design and measurement; keep scope tight and code minimal.
