# Sprint 16: io_uring I/O Pipeline

Goal: Async I/O for WAL, prefetch, and spill; batching and fixed buffers.

Acceptance:
- io_uring integrated where useful; clear wins documented vs sync.
- Backpressure and batching strategies validated.

## Learning Resources

- Linux `io_uring` man pages and official documentation.
- Tutorials and blog posts on building async I/O pipelines with `io_uring`.
- OS references on traditional blocking I/O for comparison.

## Core Questions

- Which parts of your engine will benefit most from async I/O (WAL, prefetching pages, spilling hash joins, etc.)?
- How will you structure submission and completion queues and manage in‑flight requests?
- How will you handle backpressure, batching, and error conditions?
- What is your strategy for falling back to synchronous I/O when `io_uring` is not available?

## Experiments & Measurements

- Build microbenchmarks comparing synchronous vs `io_uring`‑based I/O for WAL appends and random page reads.
- Experiment with different batch sizes and submission strategies to see their effect on latency and throughput.
- Use tracing tools (e.g., `strace`, `perf`, or `bpftrace`) to understand how your async I/O interacts with the kernel.
