# Sprint 5: WAL and Recovery

Goal: Implement write‑ahead logging, checkpoints, and crash recovery.

Acceptance:
- WAL record format and checksums defined; group commit optional.
- Crash/replay tests pass; fsck tool validates state.
- I/O profile captured; optional io_uring integration documented.

## Learning Resources

- ARIES (Write‑Ahead Logging) overview papers and summaries.
- SQLite documentation on WAL mode and checkpointing behavior.
- CMU 15‑445 lectures on logging and recovery.
- "Database Internals" chapters on logging, checkpoints, and crash recovery.

## Core Questions

- What minimum information must each log record contain to support redo (and later undo) of operations?
- Will your initial design implement physical logging, logical logging, or a hybrid approach, and why?
- How will you assign and track log sequence numbers (LSNs) in both log records and pages?
- What ordering guarantees do you need between writing log records, updating pages, and calling `fsync`/`fdatasync`?
- How will you detect and handle torn writes or partial log records after a crash?

## Experiments & Measurements

- Build a small harness that performs logged updates, forcibly kills the process at random points, and then runs recovery to validate invariants.
- Measure the cost of different fsync strategies (per operation vs batched group commit) and record their impact on p50/p99 latency.
- Simulate log file growth and implement a simple checkpointing strategy; compare recovery time with and without checkpoints.
- Use `strace` or `perf` to inspect system calls and time spent in I/O during logging and recovery.
