# Sprint 7: Logical Optimizer (Rule‑Based)

Goal: Relational algebra, filter/projection pushdown, column pruning, basic join reorder.

Acceptance:
- Rewrites applied deterministically; plan inspection tool; tests for rewrite equivalence.
- Column pruning reduces IO; measured improvement shown.

## Learning Resources

- "Architecture of a Database System" sections on query optimization.
- CMU 15‑445 lectures on logical optimization and algebraic rewrites.
- Papers and blog posts on rule‑based optimizers and relational algebra.

## Core Questions

- Which logical algebra operators will you support (projection, selection, join, aggregation, etc.)?
- What rewrite rules will you implement first (e.g., predicate pushdown, projection pruning, join commutativity/associativity)?
- How will you ensure that rewrites are semantics‑preserving and applied deterministically?
- How will your optimizer interact with statistics and cost estimates from later sprints?

## Experiments & Measurements

- Build a small set of representative queries and inspect their logical plans before and after rewrites.
- Write tests that assert equivalence of results for original vs rewritten plans.
- Count the number of operators and estimated cardinalities before and after optimization for selected queries and note improvements.
