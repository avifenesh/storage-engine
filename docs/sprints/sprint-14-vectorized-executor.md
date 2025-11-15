# Sprint 14: Vectorized Executor

Goal: Implement data chunks, selection vectors, and vectorized scan/filter/project operators as an optional accelerator alongside the row‑store iterator model.

Acceptance:
- Stable chunk size; selection vector propagation; clear API.
- Measured speedup over tuple‑at‑a‑time; p50/p99 reported.
- Tests for correctness and chunk boundary edge cases.

## Learning Resources

- MonetDB/X100 and related vectorized execution papers.
- DuckDB and PostgreSQL blog posts and talks on vectorized execution and JIT compilation.
- CMU 15‑445 lectures on execution models and operator pipelines.

## Core Questions

- What chunk size will you use for vectorized execution, and why is it a good compromise between locality and overhead?
- How will you represent data chunks so they can be backed by either row‑store pages or columnar projections?
- How do selection vectors propagate through pipelines of operators (scan → filter → project → aggregate)?
- Which operators will you vectorize first, and which remain tuple‑at‑a‑time initially?
- How will you fall back to non‑vectorized execution when data or operators are not suitable?

## Experiments & Measurements

- Implement a baseline tuple‑at‑a‑time pipeline and compare it to a vectorized version on simple filter/project workloads.
- Vary chunk size and measure throughput, CPU utilization, and cache behavior.
- Profile where time is spent within vectorized operators (branching, memory loads, function calls) and look for hotspots.
- Measure performance with and without selection vectors to understand their overhead and benefits.
