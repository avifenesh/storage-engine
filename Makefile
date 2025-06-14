# Makefile for generating assembly files from C source with ARM64 kernel-style flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare \
         -Wno-unused-function -Wno-unused-variable -Wno-format-zero-length \
         -Wdeclaration-after-statement -Wvla -std=gnu11 -fno-strict-aliasing \
         -fno-common -fshort-wchar -fno-PIE -march=armv8-a -mtune=generic \
         -DCONFIG_ARM64 -DCONFIG_64BIT -mcmodel=small \
         -falign-functions=4 -falign-jumps=1 -falign-loops=1 \
         -fstack-protector-strong -fno-delete-null-pointer-checks \
         -fno-allow-store-data-races -Wframe-larger-than=2048 -pipe \
         -fno-asynchronous-unwind-tables -fno-jump-tables -fno-strict-overflow \
         -fno-stack-check -fconserve-stack -Wimplicit-fallthrough \
         -Wno-unused-but-set-variable -Wno-unused-const-variable \
         -Wno-pointer-sign -Wno-stringop-truncation -Wno-array-bounds \
         -Wno-stringop-overflow -Wno-restrict -Wno-maybe-uninitialized \
         -Wno-packed-not-aligned -O2 -fomit-frame-pointer -g -gdwarf-4 \
         -Werror=date-time -Werror=incompatible-pointer-types \
         -Werror=designated-init -Werror=implicit-function-declaration \
         -fmacro-prefix-map=./= -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 \
         -DCONFIG_AS_CFI_SECTIONS=1

# Assembly generation flags
ASMFLAGS = -S -fverbose-asm

# Automatically find all .c files
SOURCES = $(wildcard *.c)
ASSEMBLY = $(SOURCES:.c=.s)
OBJECTS = $(SOURCES:.c=.o)
TARGETS = $(SOURCES:.c=)

# Default target - generate assembly files
.PHONY: all asm clean help

all: asm

# Generate assembly files for all C sources
asm: $(ASSEMBLY)
	@echo "✅ Generated assembly files: $(ASSEMBLY)"

# Rule to generate assembly from C source
%.s: %.c
	@echo "🔧 Generating assembly for $<..."
	$(CC) $(CFLAGS) $(ASMFLAGS) -o $@ $<

# Compile executables (optional)
%: %.c
	@echo "🔨 Compiling executable $@..."
	$(CC) $(CFLAGS) -o $@ $<

# Object files rule (optional)
%.o: %.c
	@echo "⚙️  Compiling object file $@..."
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -f $(OBJECTS) $(TARGETS) $(ASSEMBLY) *.learn.s *.O*.asm *.optimization.diff *.O*-vs-O*.diff

# Generate assembly with different optimization levels
asm-O0: CFLAGS := $(filter-out -O2,$(CFLAGS)) -O0
asm-O0: clean $(ASSEMBLY)
	@echo "✅ Generated unoptimized assembly files"

asm-O1: CFLAGS := $(filter-out -O2,$(CFLAGS)) -O1
asm-O1: clean $(ASSEMBLY)
	@echo "✅ Generated O1 optimized assembly files"

asm-O3: CFLAGS := $(filter-out -O2,$(CFLAGS)) -O3
asm-O3: clean $(ASSEMBLY)
	@echo "✅ Generated highly optimized assembly files"

# Generate assembly without debug info (cleaner output)
asm-clean: CFLAGS := $(filter-out -g -gdwarf-4,$(CFLAGS))
asm-clean: clean $(ASSEMBLY)
	@echo "✅ Generated clean assembly files (no debug info)"

# Generate educational assembly analysis
asm-learn: CFLAGS := $(filter-out -g -gdwarf-4,$(CFLAGS))
asm-learn: clean
	@echo "🎓 Generating educational assembly analysis..."
	@for file in $(SOURCES); do \
		base=$$(basename $$file .c); \
		echo ""; \
		echo "📚 Analyzing $$file for educational insights..."; \
		echo "=========================================="; \
		\
		echo "🔍 Checking for unused variables/functions..."; \
		$(CC) $(CFLAGS) -Wunused -Wunreachable-code -c $$file -o /dev/null 2>&1 | grep -E "(unused|unreachable)" || echo "  ✅ No unused code detected"; \
		\
		echo "🔍 Dead code elimination analysis (O0 vs O2)..."; \
		$(CC) $(filter-out -O2,$(CFLAGS)) -O0 -S -o $$base.O0.tmp $$file; \
		$(CC) $(CFLAGS) -S -o $$base.O2.tmp $$file; \
		wc -l $$base.O0.tmp $$base.O2.tmp | head -2 | awk '{printf "  %s: %s lines\n", $$2, $$1}'; \
		\
		echo "🔍 Function inlining analysis..."; \
		grep -c "call.*$$base" $$base.O0.tmp || true > /dev/null; \
		o0_calls=$$(grep -c "bl\|call" $$base.O0.tmp 2>/dev/null || echo "0"); \
		o2_calls=$$(grep -c "bl\|call" $$base.O2.tmp 2>/dev/null || echo "0"); \
		echo "  Function calls O0: $$o0_calls, O2: $$o2_calls"; \
		if [ "$$o2_calls" -lt "$$o0_calls" ]; then \
			echo "  ✨ Functions were likely inlined in O2"; \
		fi; \
		\
		echo "🔍 Register usage analysis..."; \
		o0_regs=$$(grep -o "x[0-9]\+\|w[0-9]\+" $$base.O0.tmp | sort -u | wc -l 2>/dev/null || echo "0"); \
		o2_regs=$$(grep -o "x[0-9]\+\|w[0-9]\+" $$base.O2.tmp | sort -u | wc -l 2>/dev/null || echo "0"); \
		echo "  Registers used O0: $$o0_regs, O2: $$o2_regs"; \
		\
		echo "🎯 Generating clean learning assembly..."; \
		$(CC) $(CFLAGS) -S -fverbose-asm -o $$base.learn.s $$file; \
		sed -i '/^[[:space:]]*\.cfi_/d; /^[[:space:]]*\.file/d; /^[[:space:]]*\.ident/d; /^[[:space:]]*\.section.*note/d; /^[[:space:]]*\.size/d; /^[[:space:]]*\.type/d' $$base.learn.s; \
		echo "  📝 Generated $$base.learn.s with educational annotations"; \
		\
		rm -f $$base.O0.tmp $$base.O2.tmp; \
		echo ""; \
	done
	@echo "✅ Educational assembly analysis complete!"
	@echo "📖 Check the *.learn.s files for annotated assembly code"

# Generate detailed optimization report with specific transformations
asm-optimize-report: CFLAGS := $(filter-out -g -gdwarf-4,$(CFLAGS))
asm-optimize-report: clean
	@echo "🔬 Generating detailed optimization reports..."
	@for file in $(SOURCES); do \
		base=$$(basename $$file .c); \
		echo ""; \
		echo "🧪 Detailed optimization analysis for $$file"; \
		echo "============================================="; \
		\
		echo "📋 Optimization passes enabled:"; \
		$(CC) $(CFLAGS) -Q --help=optimizers | grep enabled | head -10; \
		\
		echo ""; \
		echo "🔬 Specific optimizations performed:"; \
		$(CC) $(CFLAGS) -fopt-info-vec -fopt-info-inline -fopt-info-loop -fopt-info-ipa -c $$file -o /dev/null 2>&1 || echo "  No specific optimizations reported"; \
		\
		echo ""; \
		echo "📊 Generating side-by-side assembly comparison..."; \
		$(CC) $(filter-out -O2,$(CFLAGS)) -O0 -S -o $$base.O0.asm $$file; \
		$(CC) $(CFLAGS) -S -o $$base.O2.asm $$file; \
		echo "  Created $$base.O0.asm and $$base.O2.asm for comparison"; \
		diff -u $$base.O0.asm $$base.O2.asm > $$base.optimization.diff || true; \
		echo "  Created $$base.optimization.diff showing exact changes"; \
		\
		echo ""; \
		echo "📊 Memory access pattern analysis:"; \
		echo "  Load instructions O0: $$(grep -c "ldr\|ldp" $$base.O0.asm 2>/dev/null || echo "0")"; \
		echo "  Load instructions O2: $$(grep -c "ldr\|ldp" $$base.O2.asm 2>/dev/null || echo "0")"; \
		echo "  Store instructions O0: $$(grep -c "str\|stp" $$base.O0.asm 2>/dev/null || echo "0")"; \
		echo "  Store instructions O2: $$(grep -c "str\|stp" $$base.O2.asm 2>/dev/null || echo "0")"; \
		\
		echo ""; \
		echo "🎯 Control flow analysis:"; \
		echo "  Branches O0: $$(grep -c "b\." $$base.O0.asm 2>/dev/null || echo "0")"; \
		echo "  Branches O2: $$(grep -c "b\." $$base.O2.asm 2>/dev/null || echo "0")"; \
		\
		echo ""; \
		echo "💾 Stack usage analysis:"; \
		o0_stack=$$(grep "sp" $$base.O0.asm | wc -l 2>/dev/null || echo "0"); \
		o2_stack=$$(grep "sp" $$base.O2.asm | wc -l 2>/dev/null || echo "0"); \
		echo "  Stack operations O0: $$o0_stack, O2: $$o2_stack"; \
		\
		echo ""; \
		echo "🔍 Instruction count summary:"; \
		o0_total=$$(grep -v "^[[:space:]]*\." $$base.O0.asm | grep -v "^[[:space:]]*$$" | wc -l); \
		o2_total=$$(grep -v "^[[:space:]]*\." $$base.O2.asm | grep -v "^[[:space:]]*$$" | wc -l); \
		echo "  Total instructions O0: $$o0_total, O2: $$o2_total"; \
		if [ "$$o2_total" -lt "$$o0_total" ]; then \
			reduction=$$((100 * (o0_total - o2_total) / o0_total)); \
			echo "  ✨ Optimization reduced instructions by $$reduction%"; \
		fi; \
		\
		echo ""; \
	done
	@echo "✅ Detailed optimization report complete!"
	@echo "📊 Check *.optimization.diff files for exact assembly changes"
	@echo "📋 Check *.O0.asm and *.O2.asm for side-by-side comparison"

# Dynamic optimization level comparison (usage: make asm-compare LOW=0 HIGH=3)
asm-compare: CFLAGS := $(filter-out -g -gdwarf-4,$(CFLAGS))
asm-compare: 
    @if [ -z "$(LOW)" ] || [ -z "$(HIGH)" ]; then \
        echo "❌ Usage: make asm-compare LOW=<level> HIGH=<level>"; \
        echo "   Example: make asm-compare LOW=0 HIGH=2"; \
        echo "   Example: make asm-compare LOW=1 HIGH=3"; \
        echo "   Available levels: 0, 1, 2, 3, s, z, fast, g"; \
        exit 1; \
    fi
    @echo "🧹 Cleaning build artifacts..."
    @$(MAKE) clean > /dev/null 2>&1
    @if [ -n "$(SOURCES)" ]; then \
        for src in $(SOURCES); do \
            base=$$(basename $$src .c); \
            echo "⚖️  Comparing $$src: O$(LOW) vs O$(HIGH)"; \
            echo "========================================"; \
            echo "🔨 Compiling and measuring compilation time..."; \
            \
            echo "  📏 O$(LOW) compilation:"; \
            low_start=$$(date +%s.%N); \
            $(CC) $(CFLAGS) -O$(LOW) $(ASMFLAGS) $$src -o $$base.O$(LOW).asm 2>/dev/null || true; \
            $(CC) $(CFLAGS) -O$(LOW) $$src -o $$base.O$(LOW).bin 2>/dev/null || true; \
            low_end=$$(date +%s.%N); \
            low_time=$$(echo "$$low_end - $$low_start" | bc -l 2>/dev/null || echo "0"); \
            printf "    ⏱️  Compile time: %.3fs\n" $$low_time; \
            \
            echo "  📏 O$(HIGH) compilation:"; \
            high_start=$$(date +%s.%N); \
            $(CC) $(CFLAGS) -O$(HIGH) $(ASMFLAGS) $$src -o $$base.O$(HIGH).asm 2>/dev/null || true; \
            $(CC) $(CFLAGS) -O$(HIGH) $$src -o $$base.O$(HIGH).bin 2>/dev/null || true; \
            high_end=$$(date +%s.%N); \
            high_time=$$(echo "$$high_end - $$high_start" | bc -l 2>/dev/null || echo "0"); \
            printf "    ⏱️  Compile time: %.3fs\n" $$high_time; \
            \
            if [ -f $$base.O$(LOW).asm ] && [ -f $$base.O$(HIGH).asm ]; then \
                echo "📊 Generating comparison report..."; \
                diff -u $$base.O$(LOW).asm $$base.O$(HIGH).asm > $$base.O$(LOW)-vs-O$(HIGH).diff || true; \
                echo "  Created $$base.O$(LOW)-vs-O$(HIGH).diff"; \
                \
                echo "📦 Binary size analysis:"; \
                if [ -f $$base.O$(LOW).bin ] && [ -f $$base.O$(HIGH).bin ]; then \
                    low_size=$$(stat -c%s $$base.O$(LOW).bin 2>/dev/null || echo "0"); \
                    high_size=$$(stat -c%s $$base.O$(HIGH).bin 2>/dev/null || echo "0"); \
                    low_kb=$$(echo "scale=2; $$low_size / 1024" | bc -l 2>/dev/null || echo "0"); \
                    high_kb=$$(echo "scale=2; $$high_size / 1024" | bc -l 2>/dev/null || echo "0"); \
                    echo "  Binary O$(LOW): $$low_size bytes ($$low_kb KB)"; \
                    echo "  Binary O$(HIGH): $$high_size bytes ($$high_kb KB)"; \
                    if [ "$$low_size" -gt 0 ] && [ "$$high_size" -gt 0 ]; then \
                        if [ "$$high_size" -lt "$$low_size" ]; then \
                            reduction=$$(echo "scale=1; ($$low_size - $$high_size) * 100 / $$low_size" | bc -l); \
                            echo "  ➡️  O$(HIGH) reduced binary size by $$reduction%"; \
                        elif [ "$$high_size" -gt "$$low_size" ]; then \
                            increase=$$(echo "scale=1; ($$high_size - $$low_size) * 100 / $$low_size" | bc -l); \
                            echo "  ➡️  O$(HIGH) increased binary size by $$increase%"; \
                        else \
                            echo "  ➡️  Same binary size"; \
                        fi; \
                    fi; \
                else \
                    echo "  ❌ Could not analyze binary sizes"; \
                fi; \
                \
                echo "🕒 Compilation performance:"; \
                if [ "$$(echo "$$high_time > $$low_time" | bc -l 2>/dev/null)" = "1" ]; then \
                    slowdown=$$(echo "scale=1; ($$high_time - $$low_time) * 100 / $$low_time" | bc -l 2>/dev/null || echo "0"); \
                    echo "  ➡️  O$(HIGH) took $$slowdown% longer to compile"; \
                elif [ "$$(echo "$$high_time < $$low_time" | bc -l 2>/dev/null)" = "1" ]; then \
                    speedup=$$(echo "scale=1; ($$low_time - $$high_time) * 100 / $$low_time" | bc -l 2>/dev/null || echo "0"); \
                    echo "  ➡️  O$(HIGH) compiled $$speedup% faster"; \
                else \
                    echo "  ➡️  Similar compilation times"; \
                fi; \
                \
                echo "🔍 Assembly instruction analysis:"; \
                low_inst=$$(grep -c "^\s*[a-z]" $$base.O$(LOW).asm 2>/dev/null || echo "0"); \
                high_inst=$$(grep -c "^\s*[a-z]" $$base.O$(HIGH).asm 2>/dev/null || echo "0"); \
                echo "  Instructions O$(LOW): $$low_inst, O$(HIGH): $$high_inst"; \
                if [ "$$low_inst" -gt 0 ] && [ "$$high_inst" -gt 0 ]; then \
                    if [ "$$high_inst" -lt "$$low_inst" ]; then \
                        reduction=$$(( (low_inst - high_inst) * 100 / low_inst )); \
                        echo "  ➡️  O$(HIGH) reduced instructions by $$reduction%"; \
                    elif [ "$$high_inst" -gt "$$low_inst" ]; then \
                        increase=$$(( (high_inst - low_inst) * 100 / low_inst )); \
                        echo "  ➡️  O$(HIGH) increased instructions by $$increase% (possible loop unrolling)"; \
                    else \
                        echo "  ➡️  Same instruction count"; \
                    fi; \
                else \
                    echo "  ➡️  Unable to analyze (likely empty/header-only file)"; \
                fi; \
                \
                echo "🔍 Memory operations:"; \
                low_loads=$$(grep -c "\(ldr\|ldp\)" $$base.O$(LOW).asm 2>/dev/null || echo "0"); \
                high_loads=$$(grep -c "\(ldr\|ldp\)" $$base.O$(HIGH).asm 2>/dev/null || echo "0"); \
                low_stores=$$(grep -c "\(str\|stp\)" $$base.O$(LOW).asm 2>/dev/null || echo "0"); \
                high_stores=$$(grep -c "\(str\|stp\)" $$base.O$(HIGH).asm 2>/dev/null || echo "0"); \
                echo "  Loads O$(LOW): $$low_loads, O$(HIGH): $$high_loads"; \
                echo "  Stores O$(LOW): $$low_stores, O$(HIGH): $$high_stores"; \
                \
                echo "🔍 Control flow:"; \
                low_branches=$$(grep -c "\(b\.\|bl\|br\)" $$base.O$(LOW).asm 2>/dev/null || echo "0"); \
                high_branches=$$(grep -c "\(b\.\|bl\|br\)" $$base.O$(HIGH).asm 2>/dev/null || echo "0"); \
                echo "  Branches O$(LOW): $$low_branches, O$(HIGH): $$high_branches"; \
                \
                echo "🔍 Function calls:"; \
                low_calls=$$(grep -c "\sbl\s" $$base.O$(LOW).asm 2>/dev/null || echo "0"); \
                high_calls=$$(grep -c "\sbl\s" $$base.O$(HIGH).asm 2>/dev/null || echo "0"); \
                echo "  Function calls O$(LOW): $$low_calls, O$(HIGH): $$high_calls"; \
                if [ "$$low_calls" -gt "$$high_calls" ]; then \
                    inlined=$$(( low_calls - high_calls )); \
                    echo "  ➡️  O$(HIGH) likely inlined $$inlined function(s)"; \
                fi; \
            else \
                echo "❌ Failed to generate assembly for $$src"; \
            fi; \
            echo ""; \
        done; \
    else \
        echo "❌ No C source files found in current directory"; \
    fi

# Build executables (optional)
executables: $(TARGETS)
	@echo "✅ Built executables: $(TARGETS)"

# Memory safety analysis targets
.PHONY: asan valgrind memory-check memory-clean

# Define flags safe for memory analysis (remove ARM64 kernel-specific flags)
MEMORY_SAFE_CFLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare \
                     -Wno-unused-function -Wno-unused-variable -Wno-format-zero-length \
                     -Wdeclaration-after-statement -Wvla -std=gnu11 -fno-strict-aliasing \
                     -g -gdwarf-4 -Werror=date-time -Werror=incompatible-pointer-types \
                     -Werror=designated-init -Werror=implicit-function-declaration

# AddressSanitizer build and test
asan:
	@echo "🛡️  Building with AddressSanitizer..."
	@mkdir -p build/asan
	@for file in $(SOURCES); do \
		base=$$(basename $$file .c); \
		echo "Compiling $$file with ASan..."; \
		$(CC) $(MEMORY_SAFE_CFLAGS) \
			-fsanitize=address -fno-omit-frame-pointer -O1 \
			-o build/asan/$$base $$file || exit 1; \
	done
	@echo "✅ ASan build complete"
	@echo "🧪 Running ASan tests..."
	@export ASAN_OPTIONS="detect_leaks=1:abort_on_error=1:detect_stack_use_after_return=true"; \
	for exe in build/asan/*; do \
		if [ -x "$$exe" ]; then \
			echo "Testing $$exe..."; \
			timeout 30s $$exe || echo "❌ Issues detected in $$exe"; \
		fi; \
	done

# Valgrind build and test
valgrind:
	@echo "🛡️  Building for Valgrind analysis..."
	@mkdir -p build/valgrind
	@for file in $(SOURCES); do \
		base=$$(basename $$file .c); \
		echo "Compiling $$file for Valgrind..."; \
		$(CC) $(MEMORY_SAFE_CFLAGS) \
			-O0 -g3 \
			-o build/valgrind/$$base $$file || exit 1; \
	done
	@echo "✅ Valgrind build complete"
	@echo "🧪 Running Valgrind tests..."
	@for exe in build/valgrind/*; do \
		if [ -x "$$exe" ]; then \
			echo "Testing $$exe with Valgrind..."; \
			timeout 60s valgrind --tool=memcheck --leak-check=full \
				--show-leak-kinds=all --track-origins=yes \
				--error-exitcode=1 $$exe || echo "❌ Issues detected in $$exe"; \
		fi; \
	done

# Run both memory safety tools
memory-check: asan valgrind
	@echo "🎉 Memory safety analysis complete!"

# Clean memory analysis artifacts
memory-clean:
	@echo "🧹 Cleaning memory analysis artifacts..."
	@rm -rf build/

# Help target
help:
	@echo "📋 Assembly Generation Makefile"
	@echo ""
	@echo "🎯 Main targets:"
	@echo "  asm          - Generate assembly files (default)"
	@echo "  asm-O0       - Generate unoptimized assembly"
	@echo "  asm-O3       - Generate highly optimized assembly"
	@echo "  asm-clean    - Generate assembly without debug info"
	@echo "  asm-learn    - Generate educational assembly analysis (shows optimizations, unused code)"
	@echo "  asm-optimize-report - Detailed optimization analysis with exact changes"
	@echo "  asm-compare LOW=X HIGH=Y - Compare any two optimization levels"
	@echo "  executables  - Build executable files"
	@echo "  clean        - Remove all build artifacts"
	@echo ""
	@echo "🛡️  Memory safety targets:"
	@echo "  asan         - Build and test with AddressSanitizer"
	@echo "  valgrind     - Build and test with Valgrind"
	@echo "  memory-check - Run both ASan and Valgrind"
	@echo "  memory-clean - Clean memory analysis artifacts"
	@echo ""
	@echo "📁 Project info:"
	@echo "  Source files: $(SOURCES)"
	@echo "  Assembly files: $(ASSEMBLY)"
	@echo "  Executables: $(TARGETS)"
	@echo ""
	@echo "🔧 Usage examples:"
	@echo "  make         - Generate assembly files"
	@echo "  make asm-O0  - See unoptimized assembly"
	@echo "  make asm-O3  - See highly optimized assembly"
	@echo "  make asm-compare LOW=0 HIGH=2  - Compare O0 vs O2"
	@echo "  make asm-compare LOW=1 HIGH=3  - Compare O1 vs O3"
	@echo "  make asm-compare LOW=2 HIGH=s  - Compare O2 vs Os (size)"
	@echo "  make asm-compare LOW=2 HIGH=z  - Compare O2 vs Oz (aggressive size)"
	@echo "  make asm-compare LOW=3 HIGH=fast - Compare O3 vs Ofast (unsafe)"
	@echo "  make memory-check - Run full memory safety analysis"
	@echo "  make clean   - Clean up generated files"
