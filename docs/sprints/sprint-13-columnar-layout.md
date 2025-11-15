# Sprint 13: Columnar Layout and Encoding

Goal: Implement column groups (PAX), zone maps, and basic encodings (dict/RLE/bitpack) as optional projections on top of the row‑store heap.

Acceptance:
- Encoders/decoders documented and tested; mixed encodings supported.
- Zone maps pruned in scans; measurable benefit demonstrated.
- Benchmarks on encoded vs plain scans with p50/p99.

## Learning Resources

- PAX and column‑group layout papers.
- Apache Parquet and Apache Arrow documentation (encoding, statistics, and columnar layout).
- MonetDB/X100 and vectorized execution papers focusing on scan and predicate evaluation.
- "Database Internals" chapters on columnar storage and compression.

## Core Questions

- How will columnar projections relate back to the row‑store heap (for example, via TIDs or position lists)?
- Which encodings (dictionary, RLE, bit‑packing) will you start with, and for which data types?
- How will you design zone maps or other metadata to prune I/O effectively for range and equality predicates?
- What trade‑offs will you accept between write amplification (maintaining projections) and read performance gains?
- How will you keep encoded data self‑describing enough to support future schema evolution experiments?

## Experiments & Measurements

- Compare scan performance over plain row format vs columnar projections for simple predicates and aggregates.
- Measure space savings and CPU costs for each encoding on synthetic and realistic data distributions.
- Evaluate how zone maps (or similar min/max statistics) reduce the number of pages or segments that must be scanned.
- Use CPU profiling tools to determine whether your columnar design is primarily compute‑bound or memory‑bound.
