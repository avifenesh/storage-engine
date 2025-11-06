# Sprint 16: io_uring I/O Pipeline

Goal: Async I/O for WAL, prefetch, and spill; batching and fixed buffers.

Acceptance:
- io_uring integrated where useful; clear wins documented vs sync.
- Backpressure and batching strategies validated.

