# Sprint 6: SQL Parser and AST

Goal: Minimal SQL grammar and AST; error handling; EXPLAIN skeleton.

Acceptance:
- Grammar subset defined; parser builds AST; meaningful errors.
- EXPLAIN outputs logical plan skeleton.
- Tests cover representative syntax and failure cases.

## Learning Resources

- SQL language references focusing on the subset you plan to support.
- Parser generator tutorials (ANTLR, Bison, etc.), even if you implement a hand‑written recursive‑descent parser.
- CMU 15‑445 lectures on query parsing, ASTs, and logical plan representation.

## Core Questions

- Which subset of SQL do you want to support first, and what is explicitly out of scope?
- How will you represent the AST so that logical planning is straightforward and extensible?
- How will you report syntax and semantic errors in a way that is helpful for users and for debugging?
- How will you encode identifiers, types, and names to avoid ambiguities and reserved word conflicts?
- What will EXPLAIN output look like for simple queries, and how will you keep it readable as the engine grows?

## Experiments & Measurements

- Build a suite of valid and invalid SQL statements and ensure your parser and error messages behave as expected.
- Compare the complexity of hand‑written recursive‑descent parsing vs using a generator by prototyping both for a small subset.
- Measure parse time for very long or complex queries to understand scalability (even if parsing is not the main bottleneck).
