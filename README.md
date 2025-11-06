# Columnar Database Engine — Learning Project

Userspace‑first learning project to build a traditional columnar database incrementally: storage (hash + B+ tree), page/buffer + WAL, vectorized execution, and a pragmatic SQL subset.

## Structure

- `src/` — production sources organized by subsystem (e.g., `storage/hash`, `storage/btree`)
- `include/` — public headers consumed by sources and tests
- `tests/` — small standalone C tests with `main()`
- `bench/` — microbenchmarks (local, not for CI)
- `docs/` — sprint guides, roadmap, resources, and performance targets

## Build & Run

```bash
make asm            # Generate assembly for sources (learning)
make tests          # Build tests
make run-tests      # Build and run all tests
make bench          # Build microbenchmarks (e.g., hash bench)
make run-bench      # Build + run all microbenchmarks
make memory-check   # ASan + Valgrind compile and run
make format         # Apply Linux kernel style formatting
```

## Notes

- Kernel integration is optional and profile‑gated; most work stays in userspace.
- This is a learning repository: keep changes small, measurable, and documented.
