# 📋 C Fast-Track Learning Path – Kanban (Sprint-Based)

## 📅 Sprint 1: C Core Refresh & Memory Fundamentals

- Setup Makefile with proper flags and GDB integration ([makefile tutorial](https://makefiletutorial.com/), [GDB cheat sheet](https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf), [valgrind guide](https://valgrind.org/docs/manual/quick-start.html))

## 📅 Sprint 2: System Programming – IOCTL & Char Devices

- Build basic char device with miscdevice interface ([LDD3](https://lwn.net/Kernel/LDD3/), [LKMPG tutorial](https://tldp.org/LDP/lkmpg/2.6/html/x569.html))

- Implement IOCTL protocol (ADD_VECTOR, QUERY_TOP_K) ([Linux Journal guide](https://www.linuxjournal.com/article/7353), [embedtronics tutorial](https://embetronicx.com/tutorials/linux/device-drivers/ioctl-linux-driver-tutorial/))

- Build Rust user-space client to open /dev/vss and send IOCTL commands ([nix docs](https://docs.rs/nix/latest/nix/ioctl/index.html), [stackoverflow example](https://stackoverflow.com/questions/59490039/how-to-use-ioctl-in-rust))

- Build simple in-kernel vector store (fixed size) ([LDD3 memory chapters](https://lwn.net/Kernel/LDD3/))

## 📅 Sprint 3: Performance Structures – Ring Buffers & SIMD

- Implement ring buffer with FIFO eviction ([circular buffer guide](https://www.embedded.com/c-language-techniques-for-circular-buffers/), [kernel kfifo docs](https://www.kernel.org/doc/html/latest/core-api/kfifo.html))

- Study LRU cache eviction strategies ([linux-mm docs](https://linux-mm.org/PageReplacementAlgorithms), [LRU implementation](https://gist.github.com/marcosfelt/7974411))

- Implement basic cosine similarity in C ([similarity guide](https://www.geeksforgeeks.org/program-find-similarity-two-strings/))

- Implement cosine similarity with NEON SIMD intrinsics ([ARM NEON docs](https://developer.arm.com/documentation/den0018/d/), [intrinsics reference](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/intrinsics), [NEON tutorial](https://community.arm.com/arm-community-blogs/b/architectures-and-processors-blog/posts/neon-vectorization-tutorial-part-1))

## 📅 Sprint 4: Kernel Memory Management & Full Pipeline

- Study kernel memory allocators (kmalloc, slab) ([LDD3 reference](https://lwn.net/Kernel/LDD3/), [kernel memory docs](https://www.kernel.org/doc/html/latest/core-api/kmem.html))

- Add slab allocator support (optional) ([kernel slab docs](https://www.kernel.org/doc/html/latest/core-api/kmem.html))

- Evaluate Netlink socket communication (optional) ([netlink guide](https://people.kernel.org/steev/netlink-sockets), [examples](https://github.com/skx/netlink-examples))

- Study memory pressure handling and hooks (optional advanced) ([memory understanding](https://www.kernel.org/doc/gorman/html/understand/understand006.html), [memory accounting](https://docs.kernel.org/accounting/memory.html))

- Build full loop: Rust client → kernel IOCTL → cosine similarity → return IDs → fetch from Valkey ([valkey docs](https://valkey.io/docs/reference/client-libraries/))

## 🚧 In Progress

- Learn and practice function pointers (callback dispatcher) ([guide](https://www.geeksforgeeks.org/function-pointer-in-c/))

## ✅ Done

- Review pointer operations in C ([guide](https://beej.us/guide/bgc/html/split/pointers.html))

- Build dynamic array (malloc, realloc, free) ([tutorial](https://www.learn-c.org/en/Dynamic_memory))

- Study memory alignment and struct padding ([reference](https://www.geeksforgeeks.org/data-structure-alignment-in-c/))

- Build aligned struct for 128-bit vectors with NEON SIMD support ([code](128_aligned_vec.c))

