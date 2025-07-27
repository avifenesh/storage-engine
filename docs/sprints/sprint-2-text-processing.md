# Sprint 2: Text Processing Foundations

## Overview
Build the core text processing components for a search engine, including tokenization, inverted index construction, and TF-IDF scoring - all in user-space C.

**Duration**: 3-4 weeks  
**Prerequisites**: Completed Sprint 1 (C fundamentals)

---

## 🎯 Learning Objectives

1. **Master text tokenization** - Split documents into searchable terms
2. **Build inverted indexes** - Efficient data structure for text search
3. **Implement TF-IDF scoring** - Rank documents by relevance
4. **Design efficient hash tables** - Fast term lookup with collision handling
5. **Handle real-world text** - Unicode, punctuation, case normalization

---

## 📋 Sprint Issues

### Issue #27: Implement Text Tokenization and Normalization
**File**: `sprint-2-text-processing/tokenizer.c`

**Requirements**:
- Split text into individual words/tokens
- Convert to lowercase for case-insensitive search
- Remove punctuation (configurable)
- Handle basic Unicode (UTF-8)
- Filter common stopwords (the, a, an, etc.)

**Example Interface**:
```c
typedef struct {
    char* token;
    size_t position;  // Position in original text
    size_t length;
} token_t;

typedef struct {
    token_t* tokens;
    size_t count;
    size_t capacity;
} token_list_t;

// Main tokenization function
token_list_t* tokenize_text(const char* text, size_t length);

// Helper functions
void normalize_token(char* token);
bool is_stopword(const char* token);
void free_token_list(token_list_t* tokens);
```

**Success Criteria**:
- Tokenize 10MB of text in <1 second
- Correctly handle "Hello, World!" → ["hello", "world"]
- Memory-safe with no leaks (Valgrind clean)

---

### Issue #28: Build In-Memory Inverted Index
**File**: `sprint-2-text-processing/inverted_index.c`

**Requirements**:
- Hash table mapping terms to document lists
- Store document ID and term positions
- Support incremental document addition
- Efficient memory usage with dynamic growth

**Example Interface**:
```c
typedef struct posting {
    uint32_t doc_id;
    uint32_t* positions;  // Term positions in document
    uint32_t pos_count;
    struct posting* next;
} posting_t;

typedef struct {
    char* term;
    posting_t* postings;  // Linked list of documents
    uint32_t doc_freq;    // Number of documents containing term
} index_entry_t;

typedef struct {
    index_entry_t** table;  // Hash table
    size_t table_size;
    size_t term_count;
    uint32_t total_docs;
} inverted_index_t;

// Core index operations
inverted_index_t* create_index(size_t initial_size);
void add_document(inverted_index_t* index, uint32_t doc_id, token_list_t* tokens);
posting_t* search_term(inverted_index_t* index, const char* term);
void free_index(inverted_index_t* index);
```

**Success Criteria**:
- Index 100K documents without significant slowdown
- O(1) average lookup time
- Load factor maintained around 0.75

---

### Issue #29: Implement TF-IDF Scoring Algorithm
**File**: `sprint-2-text-processing/tfidf.c`

**Requirements**:
- Calculate Term Frequency (TF) for each document
- Compute Inverse Document Frequency (IDF) across corpus
- Combine into TF-IDF scores for ranking
- Efficient computation for large result sets

**Example Interface**:
```c
typedef struct {
    uint32_t doc_id;
    float score;
} scored_doc_t;

typedef struct {
    scored_doc_t* docs;
    size_t count;
} search_results_t;

// TF-IDF calculation
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