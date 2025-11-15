# Sprint 17: Observability and Benchmarks

Goal: TPC‑H subset runner, metrics, perf tooling, and regression gates.

Acceptance:
- Benchmarks produce p50/p95/p99; metrics.json updated per run.
- Basic perf/ftrace profiles included; hotspots explained.

## Learning Resources

- Guides on Linux perf, ftrace, and flamegraphs.
- Articles on designing metrics for databases (latency distributions, throughput, resource usage).
- Documentation for benchmark suites such as TPC‑H (for inspiration, even if you build a subset).

## Core Questions

- Which metrics are most important for understanding your engine (latency, throughput, cache hit rate, log flush times, etc.)?
- How will you structure metrics collection to avoid excessive overhead in hot paths?
- What benchmark workloads will you standardize on for regression testing?
- How will you store and compare historical metrics to detect regressions over time?

## Experiments & Measurements

- Design and run a small set of representative benchmarks (OLTP‑ish and analytic‑ish) and record p50/p95/p99 latencies.
- Generate flamegraphs for key workloads and identify top hotspots.
- Introduce intentional performance regressions in a safe branch and verify that your metrics and tools catch them.
