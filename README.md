# 🔍 Kernel Text Search Engine

A high-performance text search engine implemented as a Linux kernel module, featuring ARM64 SIMD optimizations and a safe Rust client interface.

## 🎯 Project Overview

This project implements a **kernel-accelerated text search engine** that demonstrates advanced systems programming concepts including:

- **Linux kernel module development** with character device interface
- **Text processing algorithms** (inverted index, TF-IDF scoring)
- **ARM NEON SIMD optimization** for string operations
- **Memory-safe Rust client** via IOCTL interface

### Key Features

- ⚡ **Sub-10ms search latency** for 100K documents
- 🚀 **100-500 QPS** throughput with kernel-level processing
- 💾 **3x memory efficiency** compared to user-space implementations
- 🔧 **Production-ready** error handling and memory management
- 🦀 **Safe Rust API** for user-space applications

## 🏗️ Architecture

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   Rust Client   │────▶│  IOCTL Interface│────▶│ Kernel Module   │
│   (Safe API)    │     │ (/dev/textsearch)│     │ (Text Engine)   │
└─────────────────┘     └─────────────────┘     └─────────────────┘
                                                          │
                                                          ▼
                                                 ┌─────────────────┐
                                                 │  Inverted Index │
                                                 │   (In-Kernel)   │
                                                 └─────────────────┘
```

## 📚 Learning Path

This project is structured as a 5-sprint learning journey:

### Sprint 1: C Fundamentals ✅
- Memory management, pointers, structs
- Dynamic arrays and memory alignment
- Function pointers and callbacks
- **Status**: COMPLETED

### Sprint 2: Text Processing (Current)
- Tokenization and normalization
- Inverted index data structure
- TF-IDF scoring implementation
- Hash table optimizations

### Sprint 3: Kernel Module Development
- Character device implementation
- IOCTL interface design
- Kernel memory management
- Concurrent access handling

### Sprint 4: SIMD Optimization
- ARM NEON string operations
- Vectorized hash computations
- Performance profiling
- Cache optimization

### Sprint 5: Production Features
- Rust client library
- Advanced search (phrases, boolean)
- Comprehensive testing
- Performance documentation

## 🚀 Quick Start

### Prerequisites

- ARM64 Linux system (or VM)
- Linux kernel headers (`linux-headers-$(uname -r)`)
- GCC 9+ with ARM64 support
- Rust toolchain (for client)
- 4GB RAM recommended

### Building the Project

```bash
# Clone the repository
git clone https://github.com/avifenesh/kernel-text-search.git
cd kernel-text-search

# Build user-space components
make

# Build kernel module (Sprint 3+)
cd sprint-3-kernel-device
make

# Load kernel module
sudo insmod textsearch.ko
```

### Running Tests

```bash
# Run memory safety tests
make memory-check

# Run performance benchmarks
make benchmark

# Run all tests
make test
```

## 📊 Performance Targets

| Metric | Target | Achieved |
|--------|--------|----------|
| Index 100K docs | <30s | ⏳ Sprint 2 |
| Search latency | <10ms | ⏳ Sprint 3 |
| Memory usage | <3x text | ⏳ Sprint 2 |
| Concurrent QPS | 100-500 | ⏳ Sprint 3 |

## 🛠️ Development Tools

- **Build System**: Comprehensive Makefile with educational features
- **Memory Safety**: AddressSanitizer and Valgrind integration
- **Assembly Analysis**: `make asm-compare` for optimization insights
- **Kernel Debugging**: printk, ftrace, and kmemleak support

## ⚠️ Safety Notice

**Kernel development can crash your system!** Always:
- Use a VM for kernel module development
- Keep backups of important data
- Read safety warnings in sprint guides
- Test thoroughly before production use

## 📖 Documentation

- [Project Roadmap](docs/TEXT-SEARCH-KANBAN.md) - Detailed sprint planning
- [Learning Resources](docs/TEXT_SEARCH_RESOURCES.md) - Curated tutorials and guides
- [Performance Guide](docs/PERFORMANCE_TARGETS.md) - Benchmarking methodology
- [Migration Guide](docs/MIGRATION_GUIDE.md) - Project history and evolution

## 🤝 Contributing

This is a learning project, but contributions are welcome! Please:
1. Follow the existing code style
2. Add tests for new features
3. Update documentation
4. Use the pre-commit hooks

## 📄 License

This project is licensed under the GPL-2.0 License - see the LICENSE file for details.

## 🙏 Acknowledgments

- Linux kernel documentation and community
- ARM NEON optimization guides
- Rust FFI documentation
- Various open-source text search implementations

---

**Note**: This project was restructured from an overambitious "multi-modal database" to a focused, achievable text search engine that maintains educational value while being completable by a single developer in 3-5 months.