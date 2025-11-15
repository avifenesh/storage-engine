# Sprint 15: SIMD & Compression Hot Paths

Goal: Vectorized predicates and joins; SIMD‑friendly codecs (dict/RLE/bitpack) and dispatch.

Acceptance:
- SIMD paths validated vs scalar; measurable speedups reported.
- Runtime dispatch implemented; encoding/decoding correctness tests.

## Learning Resources

- ARM NEON or x86 SIMD programming guides and intrinsics references.
- Papers and blog posts on SIMD selection, filtering, and joins.
- Documentation for common compression schemes (dictionary, RLE, bit‑packing).

## Core Questions

- Which predicates and operations are the best initial targets for SIMD acceleration in your engine?
- How will you design codecs so they can be used both in row‑store and columnar contexts?
- How will you fall back to scalar code paths based on CPU features, data types, or data shape?
- How will you validate correctness and numerical stability between scalar and SIMD implementations?

## Experiments & Measurements

- Benchmark scalar vs SIMD implementations on synthetic data with varying selectivity and distributions.
- Measure throughput and CPU utilization with and without compression for scan‑heavy workloads.
- Use perf or similar tools to verify that your hot loops are actually vectorized and to inspect instruction mixes.
