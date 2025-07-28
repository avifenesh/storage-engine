# Sprint 2: Hash Table Storage Engine

## Overview
This sprint focuses on building a kernel-space hash table storage engine with character device interface and ARM NEON optimization.

## Goals
- Implement kernel hash table with collision resolution
- Build `/dev/storage-hash` character device with IOCTL interface  
- Create ARM NEON hash function optimization
- Design efficient put/get/delete operations

## Key Files
- `hash_engine.c` - Core hash table implementation with collision resolution
- `hash_device.c` - Character device and IOCTL interface
- `neon_hash.c` - ARM SIMD hash function optimization
- `tests/hash_tests.c` - Unit tests for hash operations

## Getting Started
1. Read the sprint documentation: `../docs/sprints/sprint-2-hash-storage.md`
2. Start with hash table kernel implementation
3. Build and test each component incrementally
4. Run benchmarks to verify performance targets

## Building
```bash
# Compile hash storage engine
make sprint2

# Build kernel module
make sprint2-kernel

# Run tests
make test-sprint2

# Run benchmarks
make bench-sprint2
```

## Performance Targets (Raspberry Pi 4/5)
- Hash reads: >10,000 operations/second
- Hash writes: >3,000 operations/second  
- Memory usage: <150% of raw data size
- Hash collision rate: <10% with good distribution

## Resources
- [Hash Tables in C](https://benhoyt.com/writings/hash-table-in-c/)
- [Linux Kernel Module Programming](https://sysprog21.github.io/lkmpg/)
- [ARM NEON Programming](https://arm-software.github.io/acle/neon_intrinsics/advsimd.html)
- [Character Device Drivers](https://lwn.net/Kernel/LDD3/)