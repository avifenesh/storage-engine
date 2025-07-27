# GitHub Issue Transition Plan

## Overview
This document outlines the transition from the overambitious "Multi-Modal Hybrid Database" issues to the focused "Text Search Engine" issues.

---

## Current Issue State (26 Issues)

### Sprint 1: Issues #1-6 ✅ COMPLETED
**Action**: Keep as-is with "✅ ARCHIVED - Sprint 1 Complete" label
- These represent completed learning milestones
- No changes needed

### Sprint 2-5: Issues #7-26 🚫 TO BE CLOSED
**Action**: Close with explanation and link to replacement issues

---

## Issue Closing Template

Use this template when closing old issues #7-26:

```markdown
**CLOSING: Project Scope Refocus**

This issue is being closed as part of a major project restructuring. The original multi-modal database scope was overambitious for a learning project.

**Original Scope**: Multi-modal hybrid database with vector, graph, and text search
**New Scope**: Focused text search engine with kernel acceleration

**Replaced by**: Issue #[NEW_NUMBER] - [New Issue Title]
**New Focus**: Text search engine with achievable learning goals

See the updated project roadmap: [TEXT-SEARCH-KANBAN.md](../docs/TEXT-SEARCH-KANBAN.md)

Thank you for understanding. The new scope maintains technical depth while being achievable by a single developer.
```

---

## Issue Mapping Table

| Old Issue | Old Title | New Issue | New Title |
|-----------|-----------|-----------|-----------|
| #7 | Build basic char device with miscdevice | #31 | Build basic /dev/textsearch character device |
| #8 | Implement IOCTL protocol (ADD_VECTOR, QUERY_TOP_K) | #32 | Implement IOCTL interface for document operations |
| #9 | Build Rust user-space client | #38 | Build Rust client library for /dev/textsearch |
| #10 | Build simple in-kernel vector store | #33 | Add in-kernel text storage and indexing |
| #11 | Implement ring buffer with FIFO eviction | #27 | Implement text tokenization and normalization |
| #12 | Study LRU cache eviction strategies | - | (Not needed for text search) |
| #13 | Implement basic cosine similarity | #35 | ARM NEON string comparison optimization |
| #14 | Implement cosine similarity with NEON | #36 | SIMD hash computation for term processing |
| #15 | Study kernel memory allocators | #33 | (Covered in kernel device implementation) |
| #16 | Add slab allocator support | - | (Over-optimization for learning project) |
| #17 | Evaluate Netlink socket communication | - | (IOCTL sufficient for our needs) |
| #18 | Study memory pressure handling | - | (Advanced topic, not essential) |
| #19 | Build full integration loop | #41 | Performance optimization and documentation |
| #20 | Build enhanced char device | - | (Redundant with focused scope) |
| #21 | Implement enhanced IOCTL protocol | - | (Covered by #32) |
| #22 | Design unified query interface | #39 | Implement advanced search features |
| #23 | Implement lightweight inverted indexes | #28 | Build in-memory inverted index |
| #24 | Build SIMD graph traversal | - | (No graph support in new scope) |
| #25 | Implement query fusion | #29 | Implement TF-IDF scoring algorithm |
| #26 | Build full integration pipeline | #40 | Add comprehensive testing |

---

## New Issue Templates

### Sprint 2: Text Processing (Issues #27-30)

#### Issue #27: Implement text tokenization and normalization
```markdown
## Sprint 2: Text Tokenization and Normalization

### Learning Objectives
- [ ] Master C string processing
- [ ] Implement efficient tokenization
- [ ] Handle Unicode text properly

### Technical Requirements
**File to create**: `sprint-2-text-processing/tokenizer.c`
**Dependencies**: Sprint 1 completed
**Estimated time**: 8-12 hours

### Implementation Guide
1. **Basic Tokenization**
   - Split on whitespace and punctuation
   - Convert to lowercase
   - Handle UTF-8 encoding

2. **Stopword Filtering**
   - Create stopword list
   - Efficient lookup (hash set)

3. **Testing**
   - Unit tests for edge cases
   - Performance benchmarks

### Success Criteria
- [ ] Tokenize 10MB text in <1 second
- [ ] Handle Unicode correctly
- [ ] Memory-safe (Valgrind clean)
- [ ] 50K tokens/second throughput

### Learning Resources
- [C String Processing](https://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Strings.html)
- [UTF-8 in C](https://www.cprogramming.com/tutorial/unicode.html)

**Labels**: `sprint-2`, `learning`, `intermediate`
```

#### Issue #28: Build in-memory inverted index
```markdown
## Sprint 2: Inverted Index Data Structure

### Learning Objectives
- [ ] Design efficient hash table
- [ ] Implement posting lists
- [ ] Handle dynamic growth

### Technical Requirements
**File to create**: `sprint-2-text-processing/inverted_index.c`
**Dependencies**: Issue #27 (tokenizer)
**Estimated time**: 10-15 hours

### Implementation Guide
1. **Hash Table Design**
   ```c
   typedef struct {
       char *term;
       posting_t *postings;
       uint32_t doc_freq;
   } index_entry_t;
   ```

2. **Posting List Management**
   - Linked list of document IDs
   - Position tracking
   - Memory pooling

3. **Dynamic Resizing**
   - Monitor load factor
   - Rehash at 75% capacity

### Success Criteria
- [ ] O(1) average lookup time
- [ ] Index 100K documents efficiently
- [ ] Load factor maintained at 0.75
- [ ] No memory leaks

### Learning Resources
- [Hash Tables in C](https://benhoyt.com/writings/hash-table-in-c/)
- [Inverted Index Structure](https://www.geeksforgeeks.org/inverted-index/)

**Labels**: `sprint-2`, `learning`, `intermediate`
```

---

## GitHub CLI Commands

### Close Old Issues
```bash
# Close issues 7-26 with explanation
for i in {7..26}; do
  gh issue close $i --comment "CLOSING: Scope refocus to text search engine. See issue transition plan in docs/GITHUB_ISSUE_TRANSITION.md"
done
```

### Create New Issues
```bash
# Sprint 2 Issues
gh issue create --title "Implement text tokenization and normalization" \
                --body-file issues/issue-27.md \
                --label "sprint-2,learning,intermediate" \
                --milestone "Sprint 2: Text Processing"

gh issue create --title "Build in-memory inverted index data structure" \
                --body-file issues/issue-28.md \
                --label "sprint-2,learning,intermediate" \
                --milestone "Sprint 2: Text Processing"

gh issue create --title "Implement TF-IDF scoring algorithm" \
                --body-file issues/issue-29.md \
                --label "sprint-2,learning,intermediate" \
                --milestone "Sprint 2: Text Processing"

gh issue create --title "Create document storage and retrieval system" \
                --body-file issues/issue-30.md \
                --label "sprint-2,learning,intermediate" \
                --milestone "Sprint 2: Text Processing"

# Continue for Sprint 3-5...
```

---

## Project Board Update

### New Columns
1. **📚 Learning Resources** - Documentation and guides
2. **🔄 Sprint 2: Text Processing** - Current sprint
3. **📋 Sprint 3-5 Backlog** - Future sprints
4. **✅ Completed** - Finished work
5. **🚫 Archived (Old Scope)** - Closed overambitious issues

### Milestones
- Sprint 2: Text Processing (4 issues)
- Sprint 3: Kernel Device (4 issues)
- Sprint 4: SIMD Optimization (3 issues)
- Sprint 5: Production Features (4 issues)

---

## Communication Plan

### Project README Update
Add a prominent notice about the restructuring:

```markdown
## 🔄 Project Restructured (July 2025)

This project has been refocused from an overambitious "multi-modal database" to a realistic "kernel-accelerated text search engine". 

**Why the change?**
- Original scope was impossible for a learning project
- New scope maintains technical depth
- Achievable in 3-5 months by one developer
- Still covers kernel programming, SIMD, and systems design

See [MIGRATION_GUIDE.md](docs/MIGRATION_GUIDE.md) for details.
```

### Issue Comment for Active Contributors
If there were active contributors on old issues, add personal comment:

```markdown
@username - Thank you for your interest in this issue. As part of our project restructuring, we're closing this issue and replacing it with a more focused alternative. 

The new issue #XX covers similar learning objectives but with a more achievable scope. We'd love to have your contribution on the new issue if you're still interested!
```

---

## Timeline

1. **Week 1**: 
   - Close old issues #7-26
   - Create new issues #27-41
   - Update project board

2. **Week 2**:
   - Update all documentation
   - Communicate changes
   - Begin Sprint 2 work

---

## Success Metrics

- All old issues closed with clear explanations
- New issues created with detailed templates
- Project board reorganized
- Documentation updated
- No confusion about project direction

---

*Remember: Clear communication about the restructuring helps maintain project credibility and learning value.*