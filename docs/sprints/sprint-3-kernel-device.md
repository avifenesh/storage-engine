# Sprint 3: Kernel Character Device

## Overview
Port the text search engine into Linux kernel space by creating a character device with IOCTL interface. Learn kernel programming fundamentals while building `/dev/textsearch`.

**Duration**: 3-4 weeks  
**Prerequisites**: Completed Sprint 2 (text processing in user-space)

---

## 🎯 Learning Objectives

1. **Master kernel module development** - Build, load, and debug kernel modules
2. **Create character devices** - Implement file operations for device interaction
3. **Design IOCTL interfaces** - Define commands for user-kernel communication
4. **Handle kernel memory** - Use kmalloc/kfree safely with proper error handling
5. **Implement kernel synchronization** - Protect data with spinlocks and mutexes

---

## 📋 Sprint Issues

### Issue #31: Build Basic /dev/textsearch Character Device
**File**: `sprint-3-kernel-device/textsearch_dev.c`

**Requirements**:
- Create loadable kernel module (LKM)
- Register character device using miscdevice
- Implement basic file operations
- Handle module initialization and cleanup
- Add module parameters for configuration

**Example Implementation**:
```c
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "textsearch"

// File operations
static int textsearch_open(struct inode *inode, struct file *file) {
    pr_info("textsearch: device opened\n");
    return 0;
}

static int textsearch_release(struct inode *inode, struct file *file) {
    pr_info("textsearch: device closed\n");
    return 0;
}

static ssize_t textsearch_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos) {
    // Return search results
    return 0;
}

static ssize_t textsearch_write(struct file *file, const char __user *buf,
                               size_t count, loff_t *ppos) {
    // Accept documents for indexing
    return count;
}

static const struct file_operations textsearch_fops = {
    .owner = THIS_MODULE,
    .open = textsearch_open,
    .release = textsearch_release,
    .read = textsearch_read,
    .write = textsearch_write,
    .llseek = no_llseek,
};

static struct miscdevice textsearch_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &textsearch_fops,
};

// Module init/exit
static int __init textsearch_init(void) {
    int ret = misc_register(&textsearch_device);
    if (ret) {
        pr_err("Failed to register misc device\n");
        return ret;
    }
    pr_info("textsearch: module loaded\n");
    return 0;
}

static void __exit textsearch_exit(void) {
    misc_deregister(&textsearch_device);
    pr_info("textsearch: module unloaded\n");
}

module_init(textsearch_init);
module_exit(textsearch_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kernel-accelerated text search engine");
```

**Success Criteria**:
- Module loads/unloads cleanly (`insmod`/`rmmod`)
- Device appears as `/dev/textsearch`
- Basic read/write operations work
- No kernel warnings in `dmesg`

---

### Issue #32: Implement IOCTL Interface for Document Operations
**File**: `sprint-3-kernel-device/textsearch_ioctl.h`

**Requirements**:
- Define IOCTL commands for search operations
- Create data structures for user-kernel communication
- Implement command dispatch in ioctl handler
- Validate all user input for security

**IOCTL Command Definitions**:
```c
#ifndef _TEXTSEARCH_IOCTL_H
#define _TEXTSEARCH_IOCTL_H

#include <linux/ioctl.h>

#define TEXTSEARCH_MAGIC 'T'

// Document operations
struct textsearch_add_doc {
    uint32_t doc_id;
    uint32_t length;
    char *content;  // User-space pointer
};

struct textsearch_search_query {
    char *query;        // Query string
    uint32_t max_results;
    uint32_t *results;  // Array of doc IDs
    float *scores;      // TF-IDF scores
    uint32_t *count;    // Actual result count
};

struct textsearch_stats {
    uint32_t total_documents;
    uint32_t total_terms;
    uint64_t index_size_bytes;
};

// IOCTL commands
#define TEXTSEARCH_ADD_DOC    _IOW(TEXTSEARCH_MAGIC, 1, struct textsearch_add_doc)
#define TEXTSEARCH_SEARCH     _IOWR(TEXTSEARCH_MAGIC, 2, struct textsearch_search_query)
#define TEXTSEARCH_GET_STATS  _IOR(TEXTSEARCH_MAGIC, 3, struct textsearch_stats)
#define TEXTSEARCH_CLEAR_INDEX _IO(TEXTSEARCH_MAGIC, 4)

#endif
```

**IOCTL Implementation**:
```c
static long textsearch_ioctl(struct file *file, unsigned int cmd,
                           unsigned long arg) {
    int ret = 0;
    
    switch (cmd) {
    case TEXTSEARCH_ADD_DOC: {
        struct textsearch_add_doc doc;
        char *content;
        
        if (copy_from_user(&doc, (void __user *)arg, sizeof(doc)))
            return -EFAULT;
            
        // Validate input
        if (doc.length > MAX_DOC_SIZE)
            return -EINVAL;
            
        // Allocate kernel buffer
        content = kmalloc(doc.length + 1, GFP_KERNEL);
        if (!content)
            return -ENOMEM;
            
        // Copy document content
        if (copy_from_user(content, doc.content, doc.length)) {
            kfree(content);
            return -EFAULT;
        }
        content[doc.length] = '\0';
        
        // Process document (add to index)
        ret = add_document_to_index(doc.doc_id, content);
        kfree(content);
        break;
    }
    
    case TEXTSEARCH_SEARCH:
        ret = handle_search_query((void __user *)arg);
        break;
        
    case TEXTSEARCH_GET_STATS:
        ret = get_index_stats((void __user *)arg);
        break;
        
    case TEXTSEARCH_CLEAR_INDEX:
        clear_index();
        break;
        
    default:
        return -ENOTTY;
    }
    
    return ret;
}
```

**Success Criteria**:
- All IOCTL commands work correctly
- Proper error handling for invalid input
- No kernel panics with malformed data
- User-kernel data transfer verified

---

### Issue #33: Add In-Kernel Text Storage and Indexing
**File**: `sprint-3-kernel-device/kernel_index.c`

**Requirements**:
- Port inverted index to kernel space
- Use kernel memory allocation appropriately
- Implement thread-safe data structures
- Handle memory pressure gracefully

**Kernel Index Structure**:
```c
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/hashtable.h>

// Hash table for terms
#define INDEX_HASH_BITS 16
static DEFINE_HASHTABLE(term_index, INDEX_HASH_BITS);
static DEFINE_SPINLOCK(index_lock);

// Term entry in hash table
struct term_entry {
    struct hlist_node node;
    char *term;
    struct list_head postings;
    atomic_t doc_freq;
};

// Posting list entry
struct posting_entry {
    struct list_head list;
    uint32_t doc_id;
    uint32_t *positions;
    uint32_t pos_count;
};

// Document storage
struct document {
    uint32_t id;
    char *content;
    size_t length;
    atomic_t ref_count;
};

// Index operations
static int add_term_to_index(const char *term, uint32_t doc_id,
                           uint32_t position) {
    struct term_entry *entry;
    unsigned int hash = hash_string(term);
    int found = 0;
    
    spin_lock(&index_lock);
    
    // Search for existing term
    hash_for_each_possible(term_index, entry, node, hash) {
        if (strcmp(entry->term, term) == 0) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        // Create new term entry
        entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
        if (!entry) {
            spin_unlock(&index_lock);
            return -ENOMEM;
        }
        
        entry->term = kstrdup(term, GFP_ATOMIC);
        if (!entry->term) {
            kfree(entry);
            spin_unlock(&index_lock);
            return -ENOMEM;
        }
        
        INIT_LIST_HEAD(&entry->postings);
        atomic_set(&entry->doc_freq, 0);
        hash_add(term_index, &entry->node, hash);
    }
    
    // Add posting
    add_posting_to_term(entry, doc_id, position);
    
    spin_unlock(&index_lock);
    return 0;
}
```

**Memory Management**:
```c
// Memory pool for small allocations
static struct kmem_cache *posting_cache;
static struct kmem_cache *term_cache;

static int init_memory_pools(void) {
    posting_cache = kmem_cache_create("textsearch_posting",
                                     sizeof(struct posting_entry),
                                     0, SLAB_HWCACHE_ALIGN, NULL);
    if (!posting_cache)
        return -ENOMEM;
        
    term_cache = kmem_cache_create("textsearch_term",
                                  sizeof(struct term_entry),
                                  0, SLAB_HWCACHE_ALIGN, NULL);
    if (!term_cache) {
        kmem_cache_destroy(posting_cache);
        return -ENOMEM;
    }
    
    return 0;
}
```

**Success Criteria**:
- Index operations work in kernel space
- No memory leaks (check with kmemleak)
- Concurrent access handled safely
- Graceful handling of allocation failures

---

### Issue #34: Implement Basic Search Functionality via IOCTL
**File**: `sprint-3-kernel-device/kernel_search.c`

**Requirements**:
- Process search queries in kernel space
- Calculate TF-IDF scores
- Return ranked results to userspace
- Implement result pagination

**Search Implementation**:
```c
static int search_terms(const char *query, struct search_results *results) {
    char *query_copy, *token;
    struct term_entry *entry;
    struct posting_entry *posting;
    unsigned int hash;
    
    // Tokenize query
    query_copy = kstrdup(query, GFP_KERNEL);
    if (!query_copy)
        return -ENOMEM;
    
    // Simple tokenization (improve in production)
    token = strsep(&query_copy, " ");
    while (token) {
        if (strlen(token) > 0) {
            // Convert to lowercase
            str_to_lower(token);
            
            // Search for term
            hash = hash_string(token);
            
            spin_lock(&index_lock);
            hash_for_each_possible(term_index, entry, node, hash) {
                if (strcmp(entry->term, token) == 0) {
                    // Found term, process postings
                    list_for_each_entry(posting, &entry->postings, list) {
                        add_to_results(results, posting->doc_id,
                                     calculate_tfidf(posting, entry));
                    }
                    break;
                }
            }
            spin_unlock(&index_lock);
        }
        token = strsep(&query_copy, " ");
    }
    
    kfree(query_copy);
    
    // Sort results by score
    sort_results(results);
    
    return 0;
}

static long handle_search_query(void __user *arg) {
    struct textsearch_search_query query;
    struct search_results *results;
    int ret;
    
    if (copy_from_user(&query, arg, sizeof(query)))
        return -EFAULT;
    
    // Allocate results structure
    results = kzalloc(sizeof(*results), GFP_KERNEL);
    if (!results)
        return -ENOMEM;
    
    // Perform search
    ret = search_terms(query.query, results);
    if (ret < 0) {
        kfree(results);
        return ret;
    }
    
    // Copy results to user space
    ret = copy_results_to_user(&query, results);
    
    kfree(results);
    return ret;
}
```

**Success Criteria**:
- Search queries return correct results
- TF-IDF scoring matches user-space version
- Results properly ranked
- No data corruption under concurrent searches

---

## 🛠️ Implementation Guide

### Week 1: Basic Kernel Module
1. Set up kernel development environment
2. Create minimal kernel module
3. Implement miscdevice registration
4. Add basic file operations
5. Test module loading/unloading

### Week 2: IOCTL Interface
1. Design IOCTL command structure
2. Implement command handlers
3. Add user-kernel data validation
4. Test with simple user program
5. Add comprehensive error handling

### Week 3: Kernel Index
1. Port hash table to kernel
2. Implement kernel memory allocation
3. Add spinlock protection
4. Create memory pools
5. Test concurrent access

### Week 4: Search and Integration
1. Implement search algorithm
2. Add TF-IDF calculation
3. Create result ranking
4. Test with real documents
5. Optimize performance

---

## 📊 Testing Strategy

### Kernel Module Testing
```bash
# Build module
make -C /lib/modules/$(uname -r)/build M=$PWD modules

# Load module
sudo insmod textsearch.ko

# Check device
ls -l /dev/textsearch

# Monitor kernel log
sudo dmesg -w

# Unload module
sudo rmmod textsearch
```

### User-Space Test Program
```c
// test_textsearch.c
#include <fcntl.h>
#include <sys/ioctl.h>
#include "textsearch_ioctl.h"

int main() {
    int fd = open("/dev/textsearch", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    
    // Add document
    struct textsearch_add_doc doc = {
        .doc_id = 1,
        .content = "Linux kernel programming is fun",
        .length = 31
    };
    
    if (ioctl(fd, TEXTSEARCH_ADD_DOC, &doc) < 0) {
        perror("ioctl ADD_DOC");
        close(fd);
        return 1;
    }
    
    // Search
    uint32_t results[10];
    float scores[10];
    uint32_t count = 0;
    
    struct textsearch_search_query query = {
        .query = "kernel",
        .max_results = 10,
        .results = results,
        .scores = scores,
        .count = &count
    };
    
    if (ioctl(fd, TEXTSEARCH_SEARCH, &query) < 0) {
        perror("ioctl SEARCH");
        close(fd);
        return 1;
    }
    
    printf("Found %u results\n", count);
    
    close(fd);
    return 0;
}
```

### Stress Testing
```bash
# Concurrent access test
for i in {1..10}; do
    ./test_textsearch &
done
wait

# Memory leak detection
sudo sh -c 'echo scan > /sys/kernel/debug/kmemleak'
sudo cat /sys/kernel/debug/kmemleak
```

---

## 🐛 Common Issues and Solutions

### Issue: Kernel panic on module load
**Solution**: Check for NULL pointer dereferences, add proper error handling

### Issue: Deadlock with spinlocks
**Solution**: Always use spin_lock_irqsave in interrupt context, avoid nested locks

### Issue: Memory allocation failures
**Solution**: Use GFP_KERNEL in process context, handle -ENOMEM gracefully

### Issue: User data corruption
**Solution**: Always validate user pointers, use copy_from/to_user

---

## 📚 Resources

### Essential Reading
- [Linux Device Drivers, 3rd Edition](https://lwn.net/Kernel/LDD3/) - Chapters 3, 6, 8
- [Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)
- [Kernel API Documentation](https://www.kernel.org/doc/html/latest/)

### IOCTL References
- [IOCTL Implementation Guide](https://embetronicx.com/tutorials/linux/device-drivers/ioctl-tutorial-in-linux/)
- [Kernel IOCTL Best Practices](https://www.kernel.org/doc/html/latest/driver-api/ioctl.html)

### Debugging Tools
- [Debugging Kernel Modules](https://www.kernel.org/doc/html/latest/dev-tools/kgdb.html)
- [Using printk effectively](https://elinux.org/Debugging_by_printing)
- [Kernel Memory Debugging](https://www.kernel.org/doc/html/latest/dev-tools/kmemleak.html)

---

## 🔍 Kernel Debugging Guide

### Basic Debugging with printk
```c
// Debug levels
pr_debug("Debug message - compiled out unless DEBUG defined\n");
pr_info("Informational message\n");
pr_warn("Warning message\n");
pr_err("Error message\n");

// Rate limiting to avoid log spam
pr_info_ratelimited("This won't flood the logs\n");

// Conditional debugging
#define DBG_PREFIX "textsearch: "
#define dbg_print(fmt, ...) \
    pr_debug(DBG_PREFIX fmt, ##__VA_ARGS__)
```

### Using dmesg Effectively
```bash
# Watch kernel messages in real-time
sudo dmesg -w

# Show only errors and warnings
sudo dmesg -l err,warn

# Clear kernel ring buffer
sudo dmesg -c

# Show timestamps
sudo dmesg -T
```

### Dynamic Debug (dyndbg)
```bash
# Enable debug for specific module
echo 'module textsearch +p' > /sys/kernel/debug/dynamic_debug/control

# Enable debug for specific function
echo 'func textsearch_ioctl +p' > /sys/kernel/debug/dynamic_debug/control

# Enable with line numbers
echo 'module textsearch +plf' > /sys/kernel/debug/dynamic_debug/control
```

### Memory Leak Detection with kmemleak
```bash
# Enable kmemleak (add to kernel command line)
kmemleak=on

# Trigger scan
echo scan > /sys/kernel/debug/kmemleak

# View results
cat /sys/kernel/debug/kmemleak

# Clear results
echo clear > /sys/kernel/debug/kmemleak
```

### Debugging Kernel Panics
```c
// Add debug info before potential crash points
pr_info("About to access pointer %p\n", ptr);
if (!ptr) {
    pr_err("NULL pointer detected!\n");
    dump_stack();  // Print call trace
    return -EINVAL;
}

// Use BUG_ON for critical errors (causes panic)
BUG_ON(condition);

// Use WARN_ON for non-critical issues
if (WARN_ON(suspicious_condition))
    return -EINVAL;
```

### ftrace for Function Tracing
```bash
# Enable function tracer
echo function > /sys/kernel/debug/tracing/current_tracer

# Filter for your module
echo ':mod:textsearch' > /sys/kernel/debug/tracing/set_ftrace_filter

# Start tracing
echo 1 > /sys/kernel/debug/tracing/tracing_on

# View trace
cat /sys/kernel/debug/tracing/trace

# Stop tracing
echo 0 > /sys/kernel/debug/tracing/tracing_on
```

### Common Debugging Scenarios

**Scenario 1: Module won't load**
```bash
# Check exact error
sudo insmod textsearch.ko
# If fails, check dmesg
sudo dmesg | tail -20

# Common issues:
# - Symbol errors: Check MODULE_LICENSE
# - Version mismatch: Rebuild against current kernel
```

**Scenario 2: System freezes on module operation**
```c
// Add timeout to potentially blocking operations
unsigned long timeout = jiffies + HZ * 5; // 5 seconds
while (condition && time_before(jiffies, timeout)) {
    cpu_relax();
}
if (!condition) {
    pr_err("Operation timed out\n");
    return -ETIMEDOUT;
}
```

**Scenario 3: Data corruption**
```c
// Add magic numbers for corruption detection
#define MAGIC_START 0xDEADBEEF
#define MAGIC_END   0xCAFEBABE

struct my_struct {
    u32 magic_start;
    // ... actual data ...
    u32 magic_end;
};

// Check before use
if (s->magic_start != MAGIC_START || s->magic_end != MAGIC_END) {
    pr_err("Structure corruption detected!\n");
    dump_stack();
    return -EFAULT;
}
```

---

## ✅ Deliverables

By the end of Sprint 3, you should have:

1. **Working kernel module** that loads/unloads cleanly
2. **Character device** `/dev/textsearch` with file operations
3. **IOCTL interface** for all search operations
4. **In-kernel index** with proper synchronization
5. **Search functionality** matching user-space performance
6. **Test suite** for kernel module
7. **Documentation** for module usage
8. **No memory leaks** or kernel warnings

---

## 🚀 Next Sprint Preview

Sprint 4 will optimize your kernel search engine with SIMD:
- Implement ARM NEON string operations
- Vectorize hash computations
- Optimize hot paths identified by profiling
- Compare kernel vs user-space performance

Make sure your kernel module is stable before adding optimizations!

---

*Remember: Kernel programming is unforgiving. One bug can crash the entire system. Always test in a VM first!*