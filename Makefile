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

# Define compatible flags for binary compilation (removes problematic kernel flags)
BINARY_SAFE_CFLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare \
                     -Wno-unused-function -Wno-unused-variable -Wno-format-zero-length \
                     -Wdeclaration-after-statement -Wvla -std=gnu11 -fno-strict-aliasing \
                     -g -gdwarf-4 -Werror=date-time -Werror=incompatible-pointer-types \
                     -Werror=designated-init -Werror=implicit-function-declaration

# Automatically find all .c files in sprint folders
SPRINT_DIRS = sprint-1-core-memory sprint-2-system-programming sprint-3-performance-simd sprint-4-kernel-memory
SOURCES = $(wildcard $(addsuffix /*.c,$(SPRINT_DIRS)))
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
	rm -f $(OBJECTS) $(TARGETS) $(ASSEMBLY) *.learn.s *.O*.asm *.optimization.diff *.O*-vs-O*.diff *.bin
	rm -rf build/

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
	@echo "🔄 Comparing optimization levels O$(LOW) vs O$(HIGH)..."
	@for file in $(SOURCES); do \
		base=$$(basename $$file .c); \
		echo ""; \
		echo "⚖️  Comparing $$file: O$(LOW) vs O$(HIGH)"; \
		echo "========================================"; \
		echo "🔨 Compiling assembly with kernel flags..."; \
		$(CC) $(CFLAGS) -O$(LOW) -S -o $$base.O$(LOW).asm $$file; \
		$(CC) $(CFLAGS) -O$(HIGH) -S -o $$base.O$(HIGH).asm $$file; \
		echo "🔨 Compiling binaries with compatible flags..."; \
		$(CC) $(BINARY_SAFE_CFLAGS) -O$(LOW) -o $$base.O$(LOW).bin $$file 2>/dev/null || echo "  ⚠️  O$(LOW) binary compilation failed"; \
		$(CC) $(BINARY_SAFE_CFLAGS) -O$(HIGH) -o $$base.O$(HIGH).bin $$file 2>/dev/null || echo "  ⚠️  O$(HIGH) binary compilation failed"; \
		echo "📊 Generating comparison report..."; \
		diff -u $$base.O$(LOW).asm $$base.O$(HIGH).asm > $$base.O$(LOW)-vs-O$(HIGH).diff || true; \
		echo "  Created $$base.O$(LOW)-vs-O$(HIGH).diff"; \
		echo ""; \
		echo "📦 Binary Size Analysis:"; \
		if [ -f $$base.O$(LOW).bin ] && [ -f $$base.O$(HIGH).bin ]; then \
			low_size=$$(stat -c%s $$base.O$(LOW).bin 2>/dev/null || echo "0"); \
			high_size=$$(stat -c%s $$base.O$(HIGH).bin 2>/dev/null || echo "0"); \
			low_kb=$$(echo "scale=2; $$low_size / 1024" | bc -l 2>/dev/null || echo "0"); \
			high_kb=$$(echo "scale=2; $$high_size / 1024" | bc -l 2>/dev/null || echo "0"); \
			echo "  📏 O$(LOW) binary: $$low_size bytes ($$low_kb KB)"; \
			echo "  📏 O$(HIGH) binary: $$high_size bytes ($$high_kb KB)"; \
			if [ "$$low_size" -gt 0 ] && [ "$$high_size" -gt 0 ]; then \
				if [ "$$high_size" -lt "$$low_size" ]; then \
					reduction=$$(echo "scale=1; ($$low_size - $$high_size) * 100 / $$low_size" | bc -l 2>/dev/null || echo "0"); \
					saved=$$(( low_size - high_size )); \
					echo "  ✨ O$(HIGH) reduced binary size by $$reduction% (saved $$saved bytes)"; \
				elif [ "$$high_size" -gt "$$low_size" ]; then \
					increase=$$(echo "scale=1; ($$high_size - $$low_size) * 100 / $$low_size" | bc -l 2>/dev/null || echo "0"); \
					added=$$(( high_size - low_size )); \
					echo "  📈 O$(HIGH) increased binary size by $$increase% (added $$added bytes)"; \
				else \
					echo "  ➡️  Same binary size"; \
				fi; \
			fi; \
			echo ""; \
			echo "🔍 Detailed Size Breakdown:"; \
			if command -v size >/dev/null 2>&1; then \
				echo "  O$(LOW) sections:"; \
				size $$base.O$(LOW).bin | tail -n +2 | awk '{printf "    text: %s, data: %s, bss: %s, total: %s\n", $$1, $$2, $$3, $$4}'; \
				echo "  O$(HIGH) sections:"; \
				size $$base.O$(HIGH).bin | tail -n +2 | awk '{printf "    text: %s, data: %s, bss: %s, total: %s\n", $$1, $$2, $$3, $$4}'; \
			fi; \
		else \
			echo "  ❌ Could not analyze binary sizes (compilation failed)"; \
			echo "  ℹ️  Assembly analysis still available in .asm files"; \
		fi; \
		echo ""; \
		echo "⏱️  Runtime Performance Test:"; \
		if [ -f $$base.O$(LOW).bin ] && [ -f $$base.O$(HIGH).bin ]; then \
			echo "  🏃 Testing performance (1,000 iterations)..."; \
			echo "  📊 O$(LOW) timing:"; \
			low_time=$$(bash -c "time -p sh -c 'for i in \$$(seq 1 1000); do ./$$base.O$(LOW).bin >/dev/null 2>&1; done' 2>&1 | grep real | awk '{print \$$2}'" || echo "0"); \
			echo "  📊 O$(HIGH) timing:"; \
			high_time=$$(bash -c "time -p sh -c 'for i in \$$(seq 1 1000); do ./$$base.O$(HIGH).bin >/dev/null 2>&1; done' 2>&1 | grep real | awk '{print \$$2}'" || echo "0"); \
			echo "  📊 Performance Results:"; \
			printf "    O$(LOW): %.4fs (1k runs)\n" $$low_time; \
			printf "    O$(HIGH): %.4fs (1k runs)\n" $$high_time; \
			if [ "$$(echo "$$low_time > $$high_time && $$high_time > 0.001" | bc -l 2>/dev/null)" = "1" ]; then \
				speedup=$$(echo "scale=1; ($$low_time - $$high_time) * 100 / $$low_time" | bc -l 2>/dev/null || echo "0"); \
				ratio=$$(echo "scale=2; $$low_time / $$high_time" | bc -l 2>/dev/null || echo "1"); \
				echo "  ✨ O$(HIGH) is $$speedup% faster ($$ratio x speedup)"; \
			elif [ "$$(echo "$$high_time > $$low_time && $$low_time > 0.001" | bc -l 2>/dev/null)" = "1" ]; then \
				slowdown=$$(echo "scale=1; ($$high_time - $$low_time) * 100 / $$low_time" | bc -l 2>/dev/null || echo "0"); \
				ratio=$$(echo "scale=2; $$high_time / $$low_time" | bc -l 2>/dev/null || echo "1"); \
				echo "  📉 O$(HIGH) is $$slowdown% slower ($$ratio x slower)"; \
			else \
				echo "  ➡️  Performance difference too small to measure reliably"; \
			fi; \
		else \
			echo "  ❌ Cannot test runtime (binaries not available)"; \
			echo "  ℹ️  Try with files that compile to executables"; \
		fi; \
		echo "🔍 Assembly Instruction Analysis:"; \
		low_total=$$(grep -v "^[[:space:]]*\." $$base.O$(LOW).asm | grep -v "^[[:space:]]*$$" | wc -l); \
		high_total=$$(grep -v "^[[:space:]]*\." $$base.O$(HIGH).asm | grep -v "^[[:space:]]*$$" | wc -l); \
		echo "  Instructions O$(LOW): $$low_total, O$(HIGH): $$high_total"; \
		if [ "$$high_total" -lt "$$low_total" ]; then \
			reduction=$$((100 * (low_total - high_total) / low_total)); \
			echo "  ✨ O$(HIGH) reduced instructions by $$reduction%"; \
		elif [ "$$high_total" -gt "$$low_total" ]; then \
			increase=$$((100 * (high_total - low_total) / low_total)); \
			echo "  📈 O$(HIGH) increased instructions by $$increase% (possibly loop unrolling)"; \
		else \
			echo "  ➡️  Same instruction count"; \
		fi; \
		echo ""; \
	done
	@echo "✅ Optimization level comparison complete!"
	@echo "📊 Check *.O$(LOW)-vs-O$(HIGH).diff files for detailed changes"

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
