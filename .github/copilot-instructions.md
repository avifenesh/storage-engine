# Copilot instructions for this repo

This repository is a learning project for a kernel-accelerated storage/text search engine, implemented in C and organized by sprints. Use these rules to be productive immediately.

## Big picture
- Architecture is split by sprints under `sprint-*`:
  - `sprint-2-hash-storage/`: fixed-bucket KV hash engine (userspace stub), SipHash-based indexing, optional NEON helpers.
  - `sprint-3-btree-storage/`: B+ tree scaffolding for sorted/range queries.
  - `sprint-5-production-features/`: durability and infra primitives: `wal.*`, `page_cache.*`, plus simple test harness.
- Design intent favors small, composable C modules, minimal dependencies, and Linux kernel style.
- Many files are stubs-by-design; keep changes incremental and educational, not “complete product” rewrites.

## Build, test, debug
- Primary entry is the root `Makefile` with ARM64 kernel-style flags for assembly exploration and separate safe flags for binaries.
- Common tasks (run in repo root):
  ```bash
  make                 # Generate assembly for all C sources (default)
  make test-smoke      # Build+run basic Sprint 2/3 smoke tests
  make test-sprint5    # Build+run Sprint 5 test harness
  make asan            # AddressSanitizer compile+run (auto handles libs vs mains)
  make valgrind        # Valgrind compile+run
  make qa              # Aggregated quick checks (tests + format-check)
  make format          # Apply Linux kernel style from .clang-format
  make format-check    # Verify formatting; used by hooks/CI
  ```
- When comparing optimizations or learning from assembly, prefer:
  ```bash
  make asm-learn
  make asm-optimize-report
  make asm-compare LOW=0 HIGH=2
  ```
- Reports/metrics: `make report` runs `tools/generate_report.sh` over `metrics.json` (see `tools/metrics_template.json`).

## Project conventions
- Style: Linux kernel formatting; tabs with width 8, column limit ~80. Use `make format` and `make install-hooks` to install the pre-commit formatter gate.
- Target: ARM64. The default `CFLAGS` are kernel-like and may be unsuitable for final binaries. For runnable binaries, the Makefile automatically switches to safe flags in testing targets; mirror that if adding new make targets.
- No external libs. Stick to C standard library; prefer portability and small helpers over dependencies.
- Error handling: return 0 on success, negative values on failure; prefer simple errno-like codes.
- Threading: `hash_engine` uses a coarse `pthread_mutex_t` for safety. Keep APIs safe for concurrent calls unless explicitly noted.

## Module-specific patterns
- Hash engine (`sprint-2-hash-storage/`)
  - Public API in `hash_engine.h`. Keys/values are caller-owned; the engine stores raw pointers only. Do not deep-copy buffers unless adding an opt-in API.
  - Hashing: `hash_engine_hash()` uses SipHash keys (see `siphash.*`); keys are initialized in `hash_engine_init()`.
  - Current implementation is intentionally naive: single entry per bucket, no collision resolution. If you add chaining, update `struct hash_bucket` usage and maintain API contracts and stats.
- SIMD helpers (`neon_hash.c`, `neon_compare.c`): scalar placeholders today; add NEON variants behind clear, testable fallbacks. Keep scalar path correct and the default.
- B+ tree (`sprint-3-btree-storage/`): stubs outline `btree_insert/search/delete`. Implement userspace first, keep node layout and invariants easy to validate.
- Durability/cache (`sprint-5-production-features/`)
  - WAL: see `wal.h` (packed record header, append/flush/replay). Maintain forward-only LSNs and idempotent replay via callbacks.
  - Page cache: see `page_cache.h`. Pages have pins/dirty flags; never evict pinned pages; expose a `flush_dirty` callback.

## How to extend safely
- Add tests under existing harnesses: `sprint-5-production-features/test_suite.c` for C unit tests; `sprint-2-hash-storage/tests/` for hash-specific tests.
- Keep APIs minimal and document ownership, thread-safety, and complexity in headers (match existing comment style).
- Preserve educational targets in the Makefile; if you add new ones, provide learning output similar to `asm-learn`/`asm-optimize-report`.

## Examples from this repo
- Caller-owned memory pattern: `hash_engine.h` documents that `hash_put/get/delete` never copy buffers; `hash_engine.c` updates `item_count/total_memory` under `engine_lock`.
- Testing entry points: `make test-smoke`, `make test-sprint5` produce binaries in `build/` and execute them; harness prints PASS/FAIL summary.

## Editing tools note
- Some contributors use Neovim diff workflows (see `CLAUDE.md`). If available, prefer diff-based edits to keep changes reviewable. Keep commits small and focused.
