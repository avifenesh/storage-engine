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
	rm -f $(OBJECTS) $(TARGETS) $(ASSEMBLY) *.learn.s

# Generate assembly with different optimization levels
asm-O0: CFLAGS := $(filter-out -O2,$(CFLAGS)) -O0
asm-O0: clean $(ASSEMBLY)
	@echo "✅ Generated unoptimized assembly files"

asm-O3: CFLAGS := $(filter-out -O2,$(CFLAGS)) -O3
asm-O3: clean $(ASSEMBLY)
	@echo "✅ Generated highly optimized assembly files"

# Generate assembly without debug info (cleaner output)
asm-clean: CFLAGS := $(filter-out -g -gdwarf-4,$(CFLAGS))
asm-clean: clean $(ASSEMBLY)
	@echo "✅ Generated clean assembly files (no debug info)"

# Generate learning-friendly assembly (minimal verbose output)
asm-learn: CFLAGS := $(filter-out -g -gdwarf-4,$(CFLAGS))
asm-learn: ASMFLAGS := -S
asm-learn: clean
	@echo "🎓 Generating learning-friendly assembly files..."
	@for file in $(SOURCES); do \
		base=$$(basename $$file .c); \
		echo "Generating minimal assembly for $$file..."; \
		$(CC) $(CFLAGS) $(ASMFLAGS) -o $$base.learn.s $$file; \
		sed -i '/^[[:space:]]*#/d; /^[[:space:]]*$$/d; /^[[:space:]]*\.cfi_/d; /^[[:space:]]*\.file/d; /^[[:space:]]*\.ident/d; /^[[:space:]]*\.section.*note/d' $$base.learn.s; \
	done
	@echo "✅ Generated learning assembly files (*.learn.s) - minimal comments and directives"

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
	@echo "  make memory-check - Run full memory safety analysis"
	@echo "  make clean   - Clean up generated files"
