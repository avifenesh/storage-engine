# Sprint 2: System Programming – IOCTL & Char Devices

## Overview
This sprint transitions from user-space C programming to kernel development fundamentals. Focus on building character device drivers with IOCTL interfaces, preparing for the vector store kernel module.

## Tasks

### Issue #7: Build basic char device with miscdevice interface
**Learning Objectives:**
- Understand Linux kernel module basics
- Implement miscdevice character driver
- Learn kernel initialization and cleanup patterns

**Key Concepts:**
```c
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

static struct file_operations vss_fops = {
    .owner = THIS_MODULE,
    .open = vss_open,
    .release = vss_release,
    .unlocked_ioctl = vss_ioctl,
};

static struct miscdevice vss_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "vss",
    .fops = &vss_fops,
};
```

**Kernel Patterns:**
- Module initialization: `module_init()`, `module_exit()`
- Error handling with cleanup goto patterns
- Proper reference counting: `try_module_get()`, `module_put()`

**Resources:**
- [LDD3 Character Drivers](https://lwn.net/Kernel/LDD3/)
- [LKMPG tutorial](https://tldp.org/LDP/lkmpg/2.6/html/x569.html)

---

### Issue #8: Implement IOCTL protocol (ADD_VECTOR, QUERY_TOP_K)
**Learning Objectives:**
- Design IOCTL command interface
- Implement safe kernel-userspace data transfer
- Handle IOCTL command validation and processing

**IOCTL Design:**
```c
#define VSS_IOC_MAGIC 'v'
#define VSS_ADD_VECTOR    _IOW(VSS_IOC_MAGIC, 1, struct vector_data)
#define VSS_QUERY_TOP_K   _IOWR(VSS_IOC_MAGIC, 2, struct query_data)
#define VSS_GET_STATS     _IOR(VSS_IOC_MAGIC, 3, struct vss_stats)

struct vector_data {
    __u32 id;
    __u32 dimension;
    float data[128]; // Max 128-dimensional vectors
};

struct query_data {
    __u32 k;
    __u32 dimension;
    float query_vector[128];
    __u32 result_ids[32]; // Top-k results
    float distances[32];
};
```

**Safety Considerations:**
- Input validation: dimension bounds, k limits
- Copy from/to user: `copy_from_user()`, `copy_to_user()`
- Error codes: `-EINVAL`, `-EFAULT`, `-ENOMEM`

**Resources:**
- [Kernel IOCTL docs](https://docs.kernel.org/driver-api/ioctl.html)
- [Embetronicx IOCTL tutorial](https://embetronicx.com/tutorials/linux/device-drivers/ioctl-linux-driver-tutorial/)

---

### Issue #9: Build Rust user-space client for /dev/vss communication
**Learning Objectives:**
- Create safe Rust wrappers for IOCTL operations
- Implement error handling with Result types
- Design user-friendly API for vector operations

**Rust IOCTL Integration:**
```rust
use nix::ioctl_readwrite;
use std::fs::OpenOptions;
use std::os::unix::io::AsRawFd;

ioctl_readwrite!(vss_add_vector, b'v', 1, VectorData);
ioctl_readwrite!(vss_query_top_k, b'v', 2, QueryData);

pub struct VssClient {
    device: std::fs::File,
}

impl VssClient {
    pub fn new() -> Result<Self, VssError> {
        let device = OpenOptions::new()
            .read(true)
            .write(true)
            .open("/dev/vss")?;
        Ok(VssClient { device })
    }
    
    pub fn add_vector(&self, id: u32, vector: &[f32]) -> Result<(), VssError> {
        // Safe IOCTL wrapper implementation
    }
}
```

**Error Handling:**
- Custom error types with `thiserror`
- Result propagation patterns
- Graceful handling of kernel errors

**Resources:**
- [nix crate IOCTL docs](https://docs.rs/nix/latest/nix/ioctl/index.html)
- [Rust IOCTL example](https://stackoverflow.com/questions/59490039/how-to-use-ioctl-in-rust)

---

### Issue #10: Build simple in-kernel vector store
**Learning Objectives:**
- Implement basic vector storage in kernel space
- Use kernel memory allocators (kmalloc)
- Create thread-safe data structures with proper locking

**Data Structure Design:**
```c
struct vector_entry {
    u32 id;
    u32 dimension;
    float *data;
    struct list_head list;
};

struct vss_store {
    struct list_head vectors;
    spinlock_t lock;
    u32 count;
    u32 max_vectors;
};
```

**Kernel Memory Management:**
- `kmalloc(size, GFP_KERNEL)` for general allocations
- `kfree()` for cleanup
- Memory pressure considerations
- Atomic operations for counters

**Synchronization:**
- `spinlock_t` for short critical sections
- `mutex` for longer operations
- RCU for read-heavy workloads (advanced)

**Resources:**
- [LDD3 Memory Management](https://lwn.net/Kernel/LDD3/)
- [Kernel synchronization primitives](https://www.kernel.org/doc/html/latest/kernel-hacking/locking.html)

## Sprint Goals
- Establish kernel development workflow
- Create functional /dev/vss character device
- Implement basic vector storage and retrieval
- Build foundation for high-performance optimizations

## Architecture Overview
```
┌─────────────────┐    IOCTL     ┌─────────────────┐
│   Rust Client   │ ───────────▶ │  Kernel Module  │
│   (User Space)  │              │   (/dev/vss)    │
└─────────────────┘              └─────────────────┘
                                          │
                                          ▼
                                 ┌─────────────────┐
                                 │  Vector Store   │
                                 │   (In-Memory)   │
                                 └─────────────────┘
```

## Next Sprint Preview
Sprint 3 will focus on performance optimizations with ring buffers, SIMD operations, and cosine similarity implementations.
