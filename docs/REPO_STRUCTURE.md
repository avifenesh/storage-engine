# Repository Structure (Target)

This project keeps sprints as documentation only. Production code is organized by subsystem.

- `src/`
  - `common/` – utilities, error handling, arena allocators
  - `storage/`
    - `hash/` – hash engine
    - `btree/` – B+ tree engine
  - `page/` – page format, buffer manager
  - `io/` – WAL, recovery, asynchronous I/O
  - `sql/` – parser, optimizer, executor
  - `simd/` – SIMD primitives and dispatch
  - `kernel/` – optional, profile‑gated integrations
- `include/` – public headers
- `tests/` – test programs with `main()`
- `bench/` – local benchmarks (not in CI)
- `docs/` – design notes and sprint guides

Migration Plan:
1) Keep current code under `sprint-*` while sprints are independent.  
2) Move stable pieces to `src/` as they integrate; update includes and the Makefile.  
3) Keep `sprint-*` for study notes only.

