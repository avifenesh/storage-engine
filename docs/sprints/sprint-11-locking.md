# Sprint 11: Locking and Latching

Goal: Lock manager (2PL or optimistic) and latch protocols for indexes.

Acceptance:
- Deadlock detection/handling; fairness documented.
- Index latching correctness under concurrent ops.

## Learning Resources

- CMU 15‑445 lectures on locking, latching, and deadlocks.
- Classic database textbooks on two‑phase locking and lock hierarchies.
- OS resources on mutexes, reader‑writer locks, and atomic operations.

## Core Questions

- What locking scheme will you use for transactions (2PL, optimistic, or a hybrid), and why?
- How will you distinguish between logical locks (on rows/tables) and physical latches (on pages/index nodes)?
- How will you detect and resolve deadlocks, or avoid them via ordering disciplines or lock levels?
- What lock granularity will you support initially (coarse‑grained vs fine‑grained), and how will you evolve it?

## Experiments & Measurements

- Construct workloads that intentionally induce lock contention and potential deadlocks; verify correctness and recovery behavior.
- Measure throughput under different lock granularities and contention patterns.
- Use profiling tools to see where time is spent waiting for locks and latches and how that changes with tuning.
