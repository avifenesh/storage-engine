# Temporary Learning Resources - Delete When Done

## 🎯 Recommended Learning Path for C Core Concepts

Based on your current progress (Issues #1-4 completed, working on Issue #5), here's the optimal learning sequence:

### **Phase 1: Void Pointers & Generic Programming**
**Why First**: Foundation for your callback dispatcher (Issue #5) and future generic vector operations

**Study Order**:
1. **Basic Void Pointer Concepts** (30 mins)
2. **Memory Allocation with void*** (20 mins) 
3. **Safe Casting Practices** (20 mins)
4. **Generic Programming Patterns** (30 mins)

### **Phase 2: Function Pointers & Callbacks**
**Why Second**: Direct application to your current Issue #5 work

**Study Order**:
1. **Function Pointer Syntax** (20 mins)
2. **Callback Implementation** (30 mins)
3. **Dispatch Tables** (40 mins)
4. **Event Systems** (30 mins)

### **Phase 3: Type Casting Safety**
**Why Third**: Critical for ARM64/kernel safety requirements

**Study Order**:
1. **Alignment Requirements** (25 mins)
2. **Kernel Safety Patterns** (35 mins)
3. **ARM NEON Casting** (30 mins)

### **Phase 4: Memory Alignment Deep Dive**
**Why Last**: Builds on all previous concepts for optimization

**Study Order**:
1. **128-bit Alignment** (30 mins)
2. **ARM64 Specifics** (40 mins)
3. **SIMD Optimization** (45 mins)

---

## 📚 Learning Resources by Topic

### **1. Void Pointers (Generic Pointers)**

#### **Primary Resources**:
- **The GNU C Programming Tutorial - void Pointers**
  - https://www.gnu.org/software/gnu-c-manual/gnu-c-manual.html#Pointer-Types
  - **Why**: Official GNU documentation, covers void* as "generic pointer type"
  - **Focus**: Section on pointer types and generic programming

- **TutorialsPoint - C void Pointers**
  - https://www.tutorialspoint.com/cprogramming/c_void_pointers.htm
  - **Why**: Comprehensive examples with malloc/calloc patterns
  - **Focus**: Why malloc() returns void* and casting practices

- **GeeksforGeeks - void Pointer in C**
  - https://www.geeksforgeeks.org/void-pointer-c-cpp/
  - **Why**: Practical examples with common pitfalls
  - **Focus**: What you can and cannot do with void*

#### **Advanced Understanding**:
- **MarkAI Code - Ultimate Guide to void Pointers**
  - https://markai.codes/c-programming/void-pointers-in-c/
  - **Why**: Real-world applications in generic programming
  - **Focus**: Generic data structures and type-agnostic functions

- **CProgramming.com - Generic Programming with void***
  - https://www.cprogramming.com/tutorial/c/lesson6.html
  - **Why**: Covers function parameters and return values
  - **Focus**: Building reusable, type-independent code

### **2. Function Pointers & Callback Patterns**

#### **Primary Resources**:
- **Unrepo Function Pointers Tutorial**
  - https://github.com/unrepo/c-function-pointers
  - **Why**: Complete tutorial with callback and dispatch examples
  - **Focus**: Dynamic dispatch and event handling patterns

- **TutorialsPoint - Function Pointers in C**
  - https://www.tutorialspoint.com/cprogramming/c_function_pointers.htm
  - **Why**: Step-by-step progression from basic to advanced
  - **Focus**: Syntax, declaration, and basic callback patterns

- **GeeksforGeeks - Function Pointer in C**
  - https://www.geeksforgeeks.org/function-pointer-in-c/
  - **Why**: Comprehensive examples with array of function pointers
  - **Focus**: Dispatch tables and dynamic function selection

#### **Advanced Patterns**:
- **C-CPP-Notes Function Pointer Patterns**
  - https://github.com/c-cpp-notes/function-pointers
  - **Why**: Advanced patterns like state machines and plugin architectures
  - **Focus**: Real-world applications in system programming

- **DotNetTutorials Event Handling with Function Pointers**
  - https://dotnettutorials.net/lesson/function-pointers-in-c/
  - **Why**: Event-driven programming patterns
  - **Focus**: Callback registration and event dispatch systems

### **3. Type Casting & Safety**

#### **Primary Resources**:
- **CPlusPlus.com - Type Casting**
  - https://cplusplus.com/doc/tutorial/typecasting/
  - **Why**: Comprehensive coverage of C-style and safe casting
  - **Focus**: When and how to cast safely

- **GeeksforGeeks - Typecasting in C**
  - https://www.geeksforgeeks.org/typecasting-c/
  - **Why**: Implicit vs explicit casting with examples
  - **Focus**: Data loss prevention and safe practices

#### **Kernel Safety**:
- **Linux Kernel Documentation - Kernel Hacking**
  - https://docs.kernel.org/process/kernel-docs.html
  - **Why**: Official kernel programming guidelines
  - **Focus**: Type safety in kernel space

- **Linux Device Drivers (LDD3) - Chapter 3**
  - https://lwn.net/Kernel/LDD3/
  - **Why**: Character device implementation patterns
  - **Focus**: Safe data exchange between user/kernel space

### **4. Memory Alignment & ARM64 Optimization**

#### **ARM64 Architecture**:
- **ARM Developer Documentation - NEON Intrinsics**
  - https://developer.arm.com/architectures/instruction-sets/intrinsics/
  - **Why**: Official ARM documentation for SIMD operations
  - **Focus**: 128-bit alignment requirements and optimization

- **ARM NEON Programmer's Guide**
  - https://developer.arm.com/documentation/den0018/a/
  - **Why**: Complete guide to NEON SIMD programming
  - **Focus**: Data alignment and memory access patterns

#### **Memory Alignment Theory**:
- **IBM Developer - Data Alignment**
  - https://developer.ibm.com/articles/pa-dalign/
  - **Why**: Deep dive into alignment theory and performance impact
  - **Focus**: Why alignment matters for performance

- **GitHub - ARM NEON Examples**
  - https://github.com/projectNe10/Ne10
  - **Why**: Real-world NEON implementation examples
  - **Focus**: Practical alignment and optimization techniques

---

## 🔗 Quick Reference Links

### **Immediate Action Items (Today)**:
1. **Start Here**: https://www.tutorialspoint.com/cprogramming/c_void_pointers.htm
2. **Then**: https://www.tutorialspoint.com/cprogramming/c_function_pointers.htm
3. **Practice**: Apply to your `vec_dispatcher.c` (Issue #5)

### **Kernel Programming (Next Sprint)**:
- **LDD3 Online**: https://lwn.net/Kernel/LDD3/
- **Kernel Docs**: https://docs.kernel.org/driver-api/
- **IOCTL Guide**: https://docs.kernel.org/driver-api/ioctl.html

### **ARM64/NEON Optimization (Sprint 3)**:
- **ARM NEON Guide**: https://developer.arm.com/documentation/den0018/a/
- **NEON Intrinsics**: https://developer.arm.com/architectures/instruction-sets/intrinsics/

---

## 📋 Study Checklist

### **Phase 1: Void Pointers** ⏱️ ~2 hours
- [ ] Read TutorialsPoint void pointer guide (30 min)
- [ ] Study GNU C manual pointer types (30 min)
- [ ] Review GeeksforGeeks examples (20 min)
- [ ] Practice with malloc/casting patterns (20 min)
- [ ] Apply to your dynamic_array.c understanding (20 min)

### **Phase 2: Function Pointers** ⏱️ ~2 hours
- [ ] Read TutorialsPoint function pointer basics (30 min)
- [ ] Study Unrepo callback patterns (40 min)
- [ ] Practice dispatch table implementation (30 min)
- [ ] Apply to vec_dispatcher.c Issue #5 (20 min)

### **Phase 3: Type Safety** ⏱️ ~1.5 hours
- [ ] Study safe casting practices (30 min)
- [ ] Review kernel safety patterns (35 min)
- [ ] Understand ARM64 alignment requirements (25 min)

### **Phase 4: Memory Alignment** ⏱️ ~2 hours
- [ ] Read IBM alignment theory (30 min)
- [ ] Study ARM NEON alignment requirements (40 min)
- [ ] Review your 128_aligned_vec.c implementation (30 min)
- [ ] Plan SIMD optimization strategies (20 min)

---

## 🎯 Connection to Your Current Work

### **Issue #5 (Current)**: Function Pointers for Callback Dispatcher
- **Direct Application**: Phase 2 resources will solve your "expression must have integral type" error
- **Key Pattern**: Dispatch table with type-specific operation functions
- **Study Focus**: TutorialsPoint + Unrepo function pointer tutorials

### **Sprint 2 Prep**: IOCTL & Char Devices  
- **Foundation**: Phase 1 (void pointers) + Phase 3 (type safety)
- **Kernel Safety**: LDD3 documentation and kernel casting patterns
- **Study Focus**: Safe data exchange between user/kernel space

### **Sprint 3 Prep**: ARM NEON Optimization
- **Foundation**: All phases, especially Phase 4 (memory alignment)
- **Performance**: 128-bit alignment for SIMD operations
- **Study Focus**: ARM NEON intrinsics and alignment requirements

---

**📝 Remember**: Delete this file once you've bookmarked the resources and completed your learning phases!

**⚡ Quick Start**: Begin with the TutorialsPoint void pointer guide to understand why your `add` function needs proper casting, then move to function pointers to fix your dispatcher pattern.