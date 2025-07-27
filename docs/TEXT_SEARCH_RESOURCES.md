# 📚 Text Search Engine - Learning Resources

## Overview
Curated resources for building a kernel-accelerated text search engine, focusing on practical implementation rather than theoretical complexity.

---

## 📖 Core Text Search & Information Retrieval

### Inverted Index Fundamentals
- **[Inverted Index Explained](https://www.geeksforgeeks.org/inverted-index/)** - Clear introduction to inverted index concepts
- **[Building Inverted Indexes in C](https://github.com/pksh-23514/Inverted-Search-Index)** - Practical C implementation with hash tables
- **[Hash Tables in C](https://benhoyt.com/writings/hash-table-in-c/)** - Essential for building efficient indexes

### TF-IDF Algorithm
- **[Understanding TF-IDF](https://www.geeksforgeeks.org/understanding-tf-idf-term-frequency-inverse-document-frequency/)** - Step-by-step explanation
- **[TF-IDF Implementation in C++](https://github.com/taozhaojie/TFIDF_cpp)** - Reference implementation
- **[Practical TF-IDF Tutorial](https://www.freecodecamp.org/news/how-to-process-textual-data-using-tf-idf-in-python-cd2bbc0a94a3/)** - Concepts applicable to C

### Text Processing
- **[C String Processing](https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Strings.html)** - Comprehensive C string handling
- **[Tokenization in C](https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm)** - Using strtok and alternatives
- **[Better String Tokenization](https://stackoverflow.com/questions/29788983/split-string-with-delimiters-in-c)** - Modern alternatives to strtok
- **[Production Tokenization without strtok](https://stackoverflow.com/questions/69280995/how-to-tokenize-string-without-using-strtok)** - Thread-safe implementations
- **[Unicode in C with UTF-8](https://www.gnu.org/software/libunistring/manual/libunistring.html)** - Proper UTF-8 handling
- **[Stopword Lists](https://github.com/igorbrigadir/stopwords)** - Common stopwords in multiple languages
- **[Practical Tokenization Example](https://github.com/benhoyt/inih)** - Real-world C parsing code

---

## 🐧 Linux Kernel Development

### ⚠️ IMPORTANT SAFETY WARNING
Kernel development can crash your system. Always:
- Use a virtual machine or dedicated test system
- Never test on production systems
- Keep backups of important data
- Understand that kernel bugs can corrupt filesystems

### Essential Books
- **[Linux Device Drivers, 3rd Edition](https://lwn.net/Kernel/LDD3/)** - ⚠️ OUTDATED (2005) but concepts still relevant
- **[Linux Kernel Development](https://www.rorylees.com/)** - Modern kernel programming (5.x kernels)
- **[The Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)** - Updated for 5.x/6.x kernels (RECOMMENDED)

### Character Device Development
- **[Writing a Simple Character Device](https://embetronicx.com/tutorials/linux/device-drivers/linux-device-driver-tutorial-part-2-first-device-driver/)** - Step-by-step tutorial
- **[Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)** - Updated for recent kernels
- **[Miscdevice Interface](https://www.kernel.org/doc/html/latest/driver-api/misc_devices.html)** - Simpler than full char devices

### IOCTL Interface Design
- **[IOCTL Tutorial](https://embetronicx.com/tutorials/linux/device-drivers/ioctl-tutorial-in-linux/)** - Comprehensive IOCTL guide
- **[Kernel IOCTL Documentation](https://www.kernel.org/doc/html/latest/driver-api/ioctl.html)** - Official kernel docs
- **[IOCTL Best Practices](https://stackoverflow.com/questions/15807846/ioctl-linux-device-driver)** - Common patterns and pitfalls

### Kernel Memory Management
- **[Kernel Memory Allocation](https://www.kernel.org/doc/html/latest/core-api/memory-allocation.html)** - kmalloc, kfree, and friends
- **[Linux Memory Management](https://www.kernel.org/doc/gorman/html/understand/)** - Deep dive into kernel memory

### Kernel Debugging Techniques
- **[printk and dmesg](https://www.kernel.org/doc/html/latest/core-api/printk-basics.html)** - Basic kernel debugging
- **[Dynamic Debug (dyndbg)](https://www.kernel.org/doc/html/latest/admin-guide/dynamic-debug-howto.html)** - Runtime debug control
- **[KGDB Setup Guide](https://www.kernel.org/doc/html/latest/dev-tools/kgdb.html)** - Source-level kernel debugging
- **[ftrace Tutorial](https://www.kernel.org/doc/html/latest/trace/ftrace.html)** - Kernel function tracing
- **[kmemleak](https://www.kernel.org/doc/html/latest/dev-tools/kmemleak.html)** - Kernel memory leak detector
- **[KASAN](https://www.kernel.org/doc/html/latest/dev-tools/kasan.html)** - Kernel Address Sanitizer

### Kernel Text Processing Examples
- **[Linux Kernel String Functions](https://www.kernel.org/doc/html/latest/core-api/kernel-api.html#string-manipulation)** - Kernel-safe string operations
- **[seq_file Interface](https://www.kernel.org/doc/html/latest/filesystems/seq_file.html)** - Safe text output from kernel
- **[Kernel Hash Tables](https://lwn.net/Articles/510202/)** - Using kernel hash tables for indexing
- **[Kernel Text Search API](https://www.kernel.org/doc/html/latest/core-api/textsearch.html)** - Built-in text search algorithms

---

## 💪 ARM NEON SIMD Optimization

### ARM NEON Basics
- **[ARM NEON Intrinsics Reference](https://arm-software.github.io/acle/neon_intrinsics/advsimd.html)** - Official maintained reference
- **[Practical NEON Tutorial](https://www.codeproject.com/Articles/5294663/Neon-Intrinsics-for-Optimized-Math-Networking-and)** - Real-world examples
- **[NEON Guide with Examples](https://github.com/thenifty/neon-guide)** - Makes ARM NEON accessible
- **[ARM NEON Examples](https://github.com/LyleLee/arm_neon_example)** - Beginner-friendly code samples
- **[Getting Started with NEON (Video)](https://developer.arm.com/Additional%20Resources/Video%20Tutorials/Getting%20Started%20with%20Neon%20Intrinsics%20on%20Android)** - ARM official video tutorial

### String Processing with NEON
- **[SIMD String Operations](https://lemire.me/blog/2021/01/29/arm-neon-programming-quick-reference/)** - Quick reference guide
- **[Vectorized String Matching](https://arxiv.org/abs/1612.01506)** - Advanced techniques
- **[NEON Optimization Examples](https://github.com/DLTcollab/sse2neon)** - SSE to NEON conversions

### Performance Measurement
- **[ARM Performance Monitoring](https://developer.arm.com/documentation/102450/latest)** - Using PMU counters
- **[Linux perf on ARM](https://www.kernel.org/doc/html/latest/arm64/perf.html)** - Profiling on ARM64

---

## 🦀 Rust Systems Programming

### Rust Basics for C Developers
- **[The Rust Book](https://doc.rust-lang.org/book/)** - Official Rust tutorial
- **[Rust for C++ Programmers](https://github.com/nrc/r4cppp)** - Transition guide
- **[Rust FFI Guide](https://doc.rust-lang.org/nomicon/ffi.html)** - Interfacing with C
- **[The Rust FFI Omnibus](http://jakegoulding.com/rust-ffi-omnibus/)** - Practical FFI examples
- **[bindgen Tutorial](https://rust-lang.github.io/rust-bindgen/)** - Auto-generate FFI bindings

### Linux System Programming in Rust
- **[nix crate documentation](https://docs.rs/nix/latest/nix/)** - Unix/Linux APIs in Rust
- **[IOCTL in Rust](https://docs.rs/nix/latest/nix/sys/ioctl/index.html)** - Safe IOCTL wrappers
- **[Rust Linux Examples](https://github.com/rust-lang/libc)** - System call examples
- **[IOCTL Examples with nix](https://stackoverflow.com/questions/41456211/how-to-call-ioctl-in-rust-and-interface-the-linux-tun-driver)** - Practical IOCTL usage
- **[ioctl-sys crate](https://github.com/codyps/ioctl)** - Alternative IOCTL bindings
- **[Linux Kernel Module in Rust](https://github.com/fishinabarrel/linux-kernel-module-rust)** - Framework for kernel modules

---

## 🔍 Search Engine Implementation

### Open Source References
- **[SQLite FTS5](https://www.sqlite.org/fts5.html)** - Full-text search in SQLite
- **[Lucene Core Concepts](https://lucene.apache.org/core/9_4_2/core/org/apache/lucene/codecs/lucene94/package-summary.html)** - Architecture insights
- **[Tantivy (Rust)](https://github.com/quickwit-oss/tantivy)** - Modern search library design

### Implementation Tutorials
- **[JavaScript TF-IDF with Inverted Index](https://www.30secondsofcode.org/js/s/tf-idf-inverted-index/)** - Clean implementation example
- **[Python Search Engine Tutorial](https://github.com/parthasm/Search-Engine-TF-IDF)** - Step-by-step guide
- **[Movie Search Engine using TF-IDF](https://medium.com/@yashdani/movie-search-engine-using-tf-idf-c119305b3d37)** - Practical project
- **[Text Search with Elasticsearch](https://mallahyari.github.io/ml_tutorial/tfidf/)** - Production approach

### Performance Optimization
- **[Writing Cache-Friendly Code](https://www.intel.com/content/www/us/en/developer/articles/technical/cache-blocking-techniques.html)** - Intel's guide
- **[Data Structure Performance](https://www.bigocheatsheet.com/)** - Complexity reference
- **[Linux Performance Tools](https://www.brendangregg.com/linuxperf.html)** - Brendan Gregg's toolkit

---

## 📝 Project-Specific Tutorials

### Week 1-2: Text Processing Basics
1. Start with [C String Processing](https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Strings.html)
2. Build a tokenizer following [this pattern](https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm)
3. Implement hash table from [Ben Hoyt's guide](https://benhoyt.com/writings/hash-table-in-c/)

### Week 3-4: Inverted Index
1. Study [inverted index structure](https://www.geeksforgeeks.org/inverted-index/)
2. Review [C++ implementation](https://github.com/JhaPrajjwal/Inverted-Index) for ideas
3. Add TF-IDF scoring using [this guide](https://www.geeksforgeeks.org/understanding-tf-idf-term-frequency-inverse-document-frequency/)

### Week 5-6: Kernel Module
1. Complete [LDD3 Chapter 3](https://lwn.net/Kernel/LDD3/) on char drivers
2. Follow [Embetronicx tutorial](https://embetronicx.com/tutorials/linux/device-drivers/)
3. Design IOCTL interface using [best practices](https://www.kernel.org/doc/html/latest/driver-api/ioctl.html)

### Week 7-8: SIMD Optimization
1. Learn [NEON basics](https://developer.arm.com/documentation/102474/latest/)
2. Study [string processing examples](https://www.codeproject.com/Articles/5294663/Neon-Intrinsics-for-Optimized-Math-Networking-and)
3. Benchmark using [ARM PMU](https://developer.arm.com/documentation/102450/latest)

---

## 🛠️ Development Tools

### Testing Frameworks
- **[Unity Test Framework](https://github.com/ThrowTheSwitch/Unity)** - Lightweight C testing framework
- **[CUnit](http://cunit.sourceforge.net/)** - Traditional xUnit-style testing
- **[Check](https://libcheck.github.io/check/)** - Unit testing with fork() isolation
- **[cmocka](https://cmocka.org/)** - Lightweight framework with mocking support
- **[Kernel Testing Guide](https://www.kernel.org/doc/html/latest/dev-tools/testing-overview.html)** - Testing kernel modules

### Debugging
- **[GDB for Kernel Debugging](https://www.kernel.org/doc/html/latest/dev-tools/gdb-kernel-debugging.html)** - Kernel debugging setup
- **[Valgrind on ARM](https://valgrind.org/docs/manual/manual-core.html)** - Memory debugging
- **[AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer)** - Built into GCC/Clang

### Performance Analysis
- **[perf Tutorial](https://perf.wiki.kernel.org/index.php/Tutorial)** - Linux profiling
- **[Flamegraphs](https://www.brendangregg.com/flamegraphs.html)** - Visualizing performance
- **[ARM Streamline](https://developer.arm.com/tools-and-software/graphics-and-gaming/arm-mobile-studio/components/streamline-performance-analyzer)** - ARM-specific profiling

---

## 📚 Additional Reading

### Classic Papers
- **[The Anatomy of a Search Engine](http://infolab.stanford.edu/~backrub/google.html)** - Original Google paper
- **[Space/Time Trade-offs in Hash Coding](https://www.cs.cornell.edu/courses/cs312/2008sp/lectures/lec20.html)** - Bloom filters
- **[Modern B-Tree Techniques](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/bw-tree-icde2013-final.pdf)** - Advanced indexing

### Online Courses
- **[CS50 Introduction to Computer Science](https://cs50.harvard.edu/)** - Strong C fundamentals
- **[Operating Systems: Three Easy Pieces](https://pages.cs.wisc.edu/~remzi/OSTEP/)** - OS concepts
- **[CMU Database Systems](https://15445.courses.cs.cmu.edu/)** - Index implementation

---

## 🎥 Video Learning Resources

### Text Search & Information Retrieval
- **Search for "TF-IDF implementation tutorial" on YouTube** - Many CS courses have recorded lectures
- **Search for "inverted index explained" on YouTube** - Visual explanations of data structures
- **Coursera/edX courses on Information Retrieval** - Often have free video content

### ARM NEON & SIMD
- **[ARM's Official NEON Video](https://developer.arm.com/Additional%20Resources/Video%20Tutorials/Getting%20Started%20with%20Neon%20Intrinsics%20on%20Android)** - Getting started guide
- **Search for "ARM NEON optimization" on YouTube** - Conference talks and tutorials
- **Search for "SIMD programming tutorial" on YouTube** - General concepts apply to NEON

### Linux Kernel Development
- **Search for "Linux kernel module tutorial" on YouTube** - Many university lectures available
- **Greg KH's kernel development talks** - Available on YouTube from various conferences
- **"Writing Linux Device Drivers" talks** - Conference recordings often available

### C Programming & Systems
- **CS50 lectures (Harvard)** - Excellent C programming fundamentals on YouTube
- **"Advanced C Programming" courses** - Many universities post lectures online
- **Conference talks on C optimization** - CppCon and similar conferences

### Note on Video Resources
While specific YouTube links change frequently, searching for these topics typically yields high-quality results from:
- University courses (MIT, Stanford, Berkeley often post lectures)
- Tech conference recordings (FOSDEM, Linux Plumbers Conference)
- Individual educators with programming channels
- Official vendor channels (ARM, Intel, Linux Foundation)

Pro tip: Sort YouTube results by "View count" or check upload date to find proven, current content.

---

*Remember: Focus on understanding and implementing core concepts rather than getting lost in theoretical complexity. The goal is to build a working system and learn by doing.*