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
	rm -f $(OBJECTS) $(TARGETS) $(ASSEMBLY)

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

# Build executables (optional)
executables: $(TARGETS)
	@echo "✅ Built executables: $(TARGETS)"

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
	@echo "📁 Project info:"
	@echo "  Source files: $(SOURCES)"
	@echo "  Assembly files: $(ASSEMBLY)"
	@echo "  Executables: $(TARGETS)"
	@echo ""
	@echo "🔧 Usage examples:"
	@echo "  make         - Generate assembly files"
	@echo "  make asm-O0  - See unoptimized assembly"
	@echo "  make asm-O3  - See highly optimized assembly"
	@echo "  make clean   - Clean up generated files"
