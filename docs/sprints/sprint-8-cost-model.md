# Sprint 8: Cost Model and Cardinality

Goal: Stats (min/max/NDV/hist), cardinality estimates, and cost‑driven plan choices.

Acceptance:
- Stats collected/updated; estimates within reasonable error on test sets.
- Cost model influences plan selection; EXPLAIN shows costs.

## Learning Resources

- CMU 15‑445 lectures on cardinality estimation and cost models.
- Papers and blog posts on histograms, NDV estimation, and sampling.
- "Database Internals" sections on statistics and cost‑based optimization.

## Core Questions

- Which statistics will you collect first (min/max, null fraction, NDV, histograms) and how often will you update them?
- How will you estimate selectivity for simple predicates and joins?
- What cost units will you use (I/O cost, CPU cost, or a combined abstract unit)?
- How will you handle stale or missing statistics, and what fallbacks will you use?

## Experiments & Measurements

- Create synthetic tables with known distributions and compare estimated vs actual cardinalities.
- Analyze how plan choices change as you vary statistics or deliberately inject estimation errors.
- Measure end‑to‑end query time for a small workload before and after introducing cost‑based decisions.
