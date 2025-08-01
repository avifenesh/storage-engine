# ARM Storage Engine Learning Project

A personal learning project to understand C systems programming, Linux kernel development, and storage engine algorithms by implementing different storage architectures from scratch.

## Overview

This is a learning project focused on:
- Linux kernel module development
- Storage engine algorithms (hash tables, B+ trees)
- ARM64 optimization with NEON SIMD
- Memory management and performance analysis

The project is organized into sprints, each implementing a different storage engine architecture.

## Project Structure

- **sprint-1-core-memory**: C fundamentals and memory management
- **sprint-2-hash-storage**: Hash table storage engine with kernel interface
- **sprint-3-btree-storage**: B+ tree storage for sorted data
- **sprint-4-simd-optimization**: ARM NEON SIMD optimization
- **sprint-5-production-features**: Performance tuning and benchmarking

## Build Instructions

```bash
make                    # Build current sprint
make memory-check       # Run memory safety tests
make format            # Format code with Linux kernel style
```

## Important Notes

This is a learning project, not production software. Code may be incomplete, suboptimal, or contain bugs as part of the learning process. Kernel modules can crash systems - always use a VM for development.