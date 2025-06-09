# 📋 C Fast-Track Learning Path – Kanban (Sprint-Based)

---

## 📅 Sprint 1: C Core Refresh & Memory Fundamentals

### 🎯 Objective: Rebuild C fluency and master memory discipline.

#### 🏁 Backlog
- [ ] Review pointer operations in C  
      🔗 https://beej.us/guide/bgc/html/split/pointers.html
- [ ] Build dynamic array (malloc, realloc, free)  
      🔗 https://www.learn-c.org/en/Dynamic_memory
- [ ] Study memory alignment and struct padding  
      🔗 https://www.geeksforgeeks.org/data-structure-alignment-in-c/
- [ ] Build aligned struct for 128-bit vectors  
      🔗 https://www.kernel.org/doc/html/latest/core-api/unlocked-mm.html
- [ ] Learn and practice function pointers (callback dispatcher)  
      🔗 https://www.geeksforgeeks.org/function-pointer-in-c/
- [ ] Setup Makefile with proper flags and GDB integration  
      🔗 https://makefiletutorial.com/  
      🔗 https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf  
      🔗 https://valgrind.org/docs/manual/quick-start.html

---

## 📅 Sprint 2: System Programming – IOCTL & Char Devices

### 🎯 Objective: Build kernel communication through IOCTL.

#### 🏁 Backlog
- [ ] Build basic char device with miscdevice interface  
      🔗 https://lwn.net/Kernel/LDD3/  
      🔗 https://tldp.org/LDP/lkmpg/2.6/html/x569.html
- [ ] Implement IOCTL protocol (ADD_VECTOR, QUERY_TOP_K)  
      🔗 https://www.linuxjournal.com/article/7353  
      🔗 https://embetronicx.com/tutorials/linux/device-drivers/ioctl-linux-driver-tutorial/
- [ ] Build Rust user-space client to open /dev/vss and send IOCTL commands  
      🔗 https://docs.rs/nix/latest/nix/ioctl/index.html  
      🔗 https://stackoverflow.com/questions/59490039/how-to-use-ioctl-in-rust
- [ ] Build simple in-kernel vector store (fixed size)  
      🔗 https://lwn.net/Kernel/LDD3/ (memory allocation chapters)

---

## 📅 Sprint 3: Performance Structures – Ring Buffers & SIMD

### 🎯 Objective: Build fast in-kernel data structures and optimize with NEON.

#### 🏁 Backlog
- [ ] Implement ring buffer with FIFO eviction  
      🔗 https://www.embedded.com/c-language-techniques-for-circular-buffers/  
      🔗 https://www.kernel.org/doc/html/latest/core-api/kfifo.html
- [ ] Study LRU cache eviction strategies  
      🔗 https://linux-mm.org/PageReplacementAlgorithms  
      🔗 https://gist.github.com/marcosfelt/7974411
- [ ] Implement basic cosine similarity in C  
      🔗 https://www.geeksforgeeks.org/program-find-similarity-two-strings/
- [ ] Implement cosine similarity with NEON SIMD intrinsics  
      🔗 https://developer.arm.com/documentation/den0018/d/  
      🔗 https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/intrinsics  
      🔗 https://community.arm.com/arm-community-blogs/b/architectures-and-processors-blog/posts/neon-vectorization-tutorial-part-1

---

## 📅 Sprint 4: Kernel Memory Management & Full Pipeline

### 🎯 Objective: Build a memory-safe, efficient, fully integrated pipeline.

#### 🏁 Backlog
- [ ] Study kernel memory allocators (kmalloc, slab)  
      🔗 https://lwn.net/Kernel/LDD3/  
      🔗 https://www.kernel.org/doc/html/latest/core-api/kmem.html
- [ ] Add slab allocator support (optional)  
      🔗 https://www.kernel.org/doc/html/latest/core-api/kmem.html
- [ ] Evaluate Netlink socket communication (optional)  
      🔗 https://people.kernel.org/steev/netlink-sockets  
      🔗 https://github.com/skx/netlink-examples
- [ ] Study memory pressure handling and hooks (optional advanced)  
      🔗 https://www.kernel.org/doc/gorman/html/understand/understand006.html  
      🔗 https://docs.kernel.org/accounting/memory.html
- [ ] Build full loop: Rust client → kernel IOCTL → cosine similarity → return IDs → fetch from Valkey  
      🔗 https://valkey.io/docs/reference/client-libraries/

---

## 🚧 In Progress
- [ ] 

---

## ✅ Done
- [ ]
