# Sprint 4: Page Format and Buffer Manager

Goal: Define page layout and build a buffer manager with replacement policy and prefetch hooks.

Acceptance:
- Page format documented; slotted pages supported where needed.
- Buffer manager with pin/unpin, eviction policy, and stats.
- Tests for concurrency and correctness; ASan/Valgrind clean.

## Learning Resources

- SQLite file format documentation — focus on page layout and B‑tree/heap pages.
- CMU 15‑445 lectures on storage, pages, and buffer management.
- "Database Internals" chapters on page formats and buffer pools.
- OS references on virtual memory and the Linux page cache (e.g., "The Linux Programming Interface").

## Core Questions

- What page size(s) will you use, and how do they relate to the OS page size and storage block size?
- Will your heap and index pages use a slotted‑page layout, fixed‑size records, or some hybrid?
- How will you encode a tuple identifier (TID) so that indexes can reference heap rows efficiently?
- What information belongs in your page header (LSN, checksum, free‑space pointer, slot count, flags)?
- Which buffer replacement policy will you start with (LRU, clock, 2Q, etc.), and why?
- How will you track and expose buffer statistics (hit rate, evictions, dirty page flushes)?

## Experiments & Measurements

- Prototype different page sizes and measure simple sequential vs random read performance using your buffer manager.
- Simulate workloads whose working sets fit in memory vs spill beyond the buffer pool and observe hit/miss behavior.
- Add debug counters or logging to trace pin/unpin patterns and eviction decisions on representative workloads.
- Use tools like `perf` to inspect cache misses and CPU time spent in buffer‑manager code.
