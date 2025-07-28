# Sprint 2: Hash-Based Storage Engine

## Overview
Implement a high-performance hash-based storage engine with kernel acceleration. Learn hash table algorithms, memory management, and kernel programming fundamentals.

**Duration**: 3-4 weeks  
**Prerequisites**: Completed Sprint 1 (C core memory fundamentals)

---

## 🎯 Learning Objectives

1. **Master hash table algorithms** - Implement efficient collision resolution strategies
2. **Build kernel storage interface** - Create character device for key-value operations
3. **Optimize for performance** - Use SIMD instructions for hashing and comparison
4. **Handle concurrent access** - Implement proper locking and synchronization
5. **Design for scalability** - Support millions of key-value pairs efficiently

---

## 📋 Sprint Issues

### Issue #27: Implement Core Hash Table Engine
**File**: `sprint-2-hash-storage/hash_engine.c`

**Requirements**:
- Build hash table with configurable size and load factor
- Implement multiple collision resolution strategies (chaining, open addressing)
- Support variable-length keys and values
- Provide thread-safe operations with read-write locks
- Handle automatic resizing when load factor exceeded

**Example Interface**:
```c
// Hash table entry
typedef struct hash_entry {
    char *key;
    char *value;
    size_t key_len;
    size_t value_len;
    uint32_t hash;
    struct hash_entry *next;  // For chaining
} hash_entry_t;

// Hash table structure
typedef struct {
    hash_entry_t **buckets;
    size_t bucket_count;
    size_t entry_count;
    double max_load_factor;
    hash_strategy_t strategy;
    pthread_rwlock_t lock;
} hash_table_t;

// Core hash operations
hash_table_t* create_hash_table(size_t initial_size, hash_strategy_t strategy);
int hash_put(hash_table_t *table, const char *key, const char *value);
char* hash_get(hash_table_t *table, const char *key);
int hash_delete(hash_table_t *table, const char *key);
void destroy_hash_table(hash_table_t *table);
```

**Success Criteria**:
- Handle 1M+ key-value pairs with <1ms average lookup time
- Maintain load factor under 0.75 with automatic resizing
- Memory-safe with no leaks (Valgrind clean)
- Thread-safe concurrent access

---

### Issue #28: Implement Kernel Character Device Interface
**File**: `sprint-2-hash-storage/hash_device.c`

**Requirements**:
- Create kernel module with character device interface
- Implement IOCTL commands for hash operations (PUT/GET/DELETE)
- Handle user-kernel data transfer safely
- Support concurrent kernel-space access

**Example Interface**:
```c
#include <linux/ioctl.h>

#define HASH_DEVICE_MAGIC 'H'

// IOCTL command structures
typedef struct {
    char *key;
    char *value;
    size_t key_len;
    size_t value_len;
} hash_put_cmd_t;

typedef struct {
    char *key;
    char *value_buffer;
    size_t key_len;
    size_t buffer_size;
    size_t value_len;  // Returned
} hash_get_cmd_t;

typedef struct {
    char *key;
    size_t key_len;
} hash_delete_cmd_t;

// IOCTL commands
#define HASH_PUT    _IOW(HASH_DEVICE_MAGIC, 1, hash_put_cmd_t)
#define HASH_GET    _IOWR(HASH_DEVICE_MAGIC, 2, hash_get_cmd_t)
#define HASH_DELETE _IOW(HASH_DEVICE_MAGIC, 3, hash_delete_cmd_t)
#define HASH_STATS  _IOR(HASH_DEVICE_MAGIC, 4, hash_stats_t)
```

**Success Criteria**:
- Kernel module loads and creates /dev/hashstore device
- IOCTL operations work correctly from userspace
- Proper error handling and memory management in kernel space
- No kernel crashes or memory leaks

---

### Issue #29: Add SIMD-Optimized Hash Functions
**File**: `sprint-2-hash-storage/neon_hash.c`

**Requirements**:
- Implement ARM NEON optimized hash functions
- Support multiple hash algorithms (FNV-1a, CRC32, xxHash)
- Vectorize string comparison operations
- Benchmark against scalar implementations

**Example Interface**:
```c
#include <arm_neon.h>

// SIMD hash function types
typedef uint32_t (*hash_func_t)(const char *data, size_t len);

// NEON-optimized hash functions
uint32_t neon_fnv1a_hash(const char *data, size_t len);
uint32_t neon_crc32_hash(const char *data, size_t len);
uint32_t neon_xxhash(const char *data, size_t len);

// SIMD string comparison
int neon_memcmp(const void *s1, const void *s2, size_t n);
int neon_strcmp(const char *s1, const char *s2);

// Hash table with SIMD optimization
hash_table_t* create_simd_hash_table(size_t initial_size, hash_func_t hash_func);
```
float calculate_tf(uint32_t term_count, uint32_t doc_length);
float calculate_idf(uint32_t doc_freq, uint32_t total_docs);
float calculate_tfidf(float tf, float idf);

// Search and scoring
search_results_t* search_query(inverted_index_t* index, 
                              const char** query_terms, 
                              size_t term_count);
void sort_results_by_score(search_results_t* results);
```

**Mathematical Formulas**:
```
TF(term, doc) = (count of term in doc) / (total terms in doc)
IDF(term) = log(total documents / documents with term)
TF-IDF(term, doc) = TF(term, doc) * IDF(term)
```

**Success Criteria**:
- Correct TF-IDF calculations verified against reference
- Score 1000 documents in <10ms
- Results properly ranked by relevance

---

### Issue #30: Create Document Storage and Retrieval System
**File**: `sprint-2-text-processing/document_store.c`

**Requirements**:
- Store document content and metadata
- Assign unique document IDs
- Support bulk loading from files
- Retrieve documents by ID for search results

**Example Interface**:
```c
typedef struct {
    uint32_t id;
    char* title;
    char* content;
    size_t length;
    uint32_t term_count;
} document_t;

typedef struct {
    document_t** documents;
    size_t count;
    size_t capacity;
    uint32_t next_id;
} document_store_t;

// Document management
document_store_t* create_document_store(size_t initial_capacity);
uint32_t add_document(document_store_t* store, const char* title, const char* content);
document_t* get_document(document_store_t* store, uint32_t doc_id);
void load_documents_from_dir(document_store_t* store, const char* directory);
void free_document_store(document_store_t* store);
```

**Success Criteria**:
- Store 100K documents efficiently
- O(1) document retrieval by ID
- Bulk loading at >1000 docs/second

---

## 🛠️ Implementation Guide

### Week 1: Tokenization
1. Start with simple ASCII tokenization using `strtok`
2. Add lowercase conversion with `tolower`
3. Implement stopword filtering with hash set
4. Add UTF-8 support for international text
5. Benchmark and optimize hot paths

### Week 2: Inverted Index
1. Implement basic hash table with chaining
2. Add posting list management
3. Implement dynamic resizing at 75% load
4. Add position tracking for phrase queries
5. Test with increasing document counts

### Week 3: TF-IDF and Search
1. Implement TF-IDF calculations
2. Build query processing pipeline
3. Add result ranking and sorting
4. Implement basic query operators
5. Create search benchmarks

### Week 4: Integration and Testing
1. Build complete indexing pipeline
2. Create test corpus from real data
3. Implement performance benchmarks
4. Fix bugs and optimize
5. Document API and usage

---

## 📊 Testing Strategy

### Unit Tests
```c
// test_tokenizer.c
void test_basic_tokenization() {
    token_list_t* tokens = tokenize_text("Hello, World!", 13);
    assert(tokens->count == 2);
    assert(strcmp(tokens->tokens[0].token, "hello") == 0);
    assert(strcmp(tokens->tokens[1].token, "world") == 0);
}

// test_index.c
void test_index_search() {
    inverted_index_t* index = create_index(1000);
    add_document(index, 1, tokenize_text("apple banana", 12));
    posting_t* results = search_term(index, "apple");
    assert(results != NULL);
    assert(results->doc_id == 1);
}
```

### Integration Tests
```bash
# Index a directory of documents
./indexer corpus/ -o index.db

# Search the index
./search index.db "information retrieval"

# Benchmark performance
./benchmark index.db queries.txt
```

### Performance Tests
- Tokenization: 50K tokens/second minimum
- Indexing: 3K documents/second minimum
- Search: <10ms for single term
- Memory: <3x raw text size

---

## 🐛 Common Issues and Solutions

### Issue: Slow tokenization
**Solution**: Use lookup tables for character classification instead of multiple function calls

### Issue: Hash table collisions
**Solution**: Use a better hash function (djb2 or murmur) and implement Robin Hood hashing

### Issue: Memory fragmentation
**Solution**: Use memory pools for small allocations (tokens, postings)

### Issue: Unicode handling
**Solution**: Use a lightweight UTF-8 library or implement basic UTF-8 decoding

---

## 📚 Resources

### Essential Reading
- [Hash Tables in C](https://benhoyt.com/writings/hash-table-in-c/)
- [Understanding TF-IDF](https://www.geeksforgeeks.org/understanding-tf-idf-term-frequency-inverse-document-frequency/)
- [C String Processing](https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Strings.html)

### Reference Implementations
- [Simple Inverted Index in C](https://github.com/pksh-23514/Inverted-Search-Index)
- [TF-IDF in C++](https://github.com/taozhaojie/TFIDF_cpp)

### Advanced Topics
- [Posting List Compression](https://nlp.stanford.edu/IR-book/html/htmledition/postings-file-compression-1.html)
- [Skip Lists for Faster Intersection](https://www.cs.cornell.edu/courses/cs4300/2014fa/lectures/03skiplist.pdf)

---

## ✅ Deliverables

By the end of Sprint 2, you should have:

1. **Working tokenizer** that handles real-world text
2. **Functional inverted index** with efficient lookups
3. **TF-IDF scoring** implementation
4. **Document storage** system
5. **Basic search functionality** with ranking
6. **Performance benchmarks** meeting targets
7. **Comprehensive tests** with >80% coverage
8. **Clear documentation** with usage examples

---

## 🚀 Next Sprint Preview

Sprint 3 will take your text search engine into kernel space:
- Create `/dev/textsearch` character device
- Design IOCTL interface for search operations
- Port inverted index to kernel memory
- Handle concurrent access with proper locking

Make sure your user-space implementation is solid before moving to kernel space!

---

*Remember: Focus on correctness first, then optimize. A working search engine is better than a fast but broken one.*