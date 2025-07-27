# Sprint 2: Text Processing Foundations

## Overview
This sprint focuses on building the core text processing components for our search engine in user-space C.

## Goals
- Implement efficient text tokenization
- Build inverted index data structure
- Create TF-IDF scoring algorithm
- Design document storage system

## Key Files
- `tokenizer.c` - Text tokenization and normalization
- `inverted_index.c` - Hash table-based inverted index
- `tfidf.c` - TF-IDF scoring implementation
- `document_store.c` - Document storage and retrieval

## Getting Started
1. Read the sprint documentation: `../docs/sprints/sprint-2-text-processing.md`
2. Start with Issue #27 (tokenization)
3. Build and test each component incrementally
4. Run benchmarks to verify performance targets

## Building
```bash
# Compile all sprint 2 files
make sprint2

# Run tests
make test-sprint2

# Run benchmarks
make bench-sprint2
```

## Performance Targets
- Tokenization: 50K tokens/second
- Indexing: 3K documents/second
- Search latency: <10ms for single term
- Memory usage: <3x raw text size

## Resources
- [Hash Tables in C](https://benhoyt.com/writings/hash-table-in-c/)
- [Understanding TF-IDF](https://www.geeksforgeeks.org/understanding-tf-idf-term-frequency-inverse-document-frequency/)
- [C String Processing](https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Strings.html)