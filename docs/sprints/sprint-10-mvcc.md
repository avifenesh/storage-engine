# Sprint 10: MVCC and Vacuum

Goal: Tuple versioning, visibility, and vacuum/GC.

Acceptance:
- Correct visibility under concurrent readers/writers in tests.
- Vacuum reclaims space safely; no lost updates.

## Learning Resources

- PostgreSQL documentation on MVCC, snapshots, and vacuum.
- CMU 15‑445 lectures on concurrency control and MVCC.
- "Database Internals" chapters on transactions and multiversioning.

## Core Questions

- How will you represent tuple versions and transaction IDs in your row‑store heap?
- What isolation level(s) will you target first (e.g., snapshot isolation, read committed), and what anomalies will you accept or forbid?
- How will readers and writers coordinate without blocking each other unnecessarily?
- How will you detect and handle write‑write conflicts and anomalies such as lost updates or write skew?

## Experiments & Measurements

- Simulate concurrent workloads with readers and writers and validate visibility rules via targeted tests.
- Measure the overhead of versioning (extra space, additional lookups) on representative workloads.
- Implement simple long‑running transactions and explore how they interact with vacuum/GC and space reuse.
