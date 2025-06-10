# Makefile with dynamic compile_commands.json generation and ARM64 kernel-style flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare \
         -Wno-unused-function -Wno-unused-variable -Wno-format-zero-length \
         -Wdeclaration-after-statement -Wvla -std=gnu11 -fno-strict-aliasing \
         -fno-common -fshort-wchar -fno-PIE -march=armv8-a -mtune=generic \
         -mgeneral-regs-only -DCONFIG_ARM64 -DCONFIG_64BIT -mcmodel=small \
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

# Automatically find all .c files
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
TARGETS = $(SOURCES:.c=)

# Default target
.PHONY: all clean compile_commands.json bear-gen js-gen watch

all: $(TARGETS) compile_commands.json

# Compile each .c file to executable
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGETS) compile_commands.json

# Generate compile_commands.json using Bear (recommended for complex builds)
bear-gen:
	@echo "🐻 Generating compile_commands.json using Bear..."
	@if command -v bear >/dev/null 2>&1; then \
		bear -- $(MAKE) $(TARGETS); \
	else \
		echo "❌ Bear not found. Install it or use 'make js-gen'"; \
		exit 1; \
	fi

# Generate compile_commands.json using JavaScript (default method)
js-gen:
	@echo "� Generating compile_commands.json using JavaScript..."
	@if command -v node >/dev/null 2>&1; then \
		node generate-compile-commands.js; \
	else \
		echo "❌ Node.js not found. Please install Node.js"; \
		exit 1; \
	fi

# Watch mode - auto-regenerate on file changes
watch:
	@echo "👀 Starting watch mode..."
	@if command -v node >/dev/null 2>&1; then \
		node generate-compile-commands.js --watch; \
	else \
		echo "❌ Node.js not found. Please install Node.js"; \
		exit 1; \
	fi

# Auto-generate compile_commands.json (prioritizes JavaScript, falls back to Bear)
compile_commands.json: $(SOURCES)
	@echo "🔄 Auto-generating compile_commands.json..."
	@if command -v node >/dev/null 2>&1 && [ -f generate-compile-commands.js ]; then \
		echo "Using JavaScript..."; \
		node generate-compile-commands.js; \
	elif command -v bear >/dev/null 2>&1; then \
		echo "Using Bear..."; \
		bear -- $(MAKE) $(TARGETS); \
	else \
		echo "❌ No generation method available. Please install Node.js or Bear"; \
		exit 1; \
	fi

# Help target
help:
	@echo "📋 Available targets:"
	@echo "  all          - Build all executables and generate compile_commands.json"
	@echo "  clean        - Remove build artifacts"
	@echo "  js-gen       - Generate compile_commands.json using JavaScript"
	@echo "  bear-gen     - Generate compile_commands.json using Bear (if available)"
	@echo "  watch        - Watch for changes and auto-regenerate compile_commands.json"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "📁 Project info:"
	@echo "  Source files: $(SOURCES)"
	@echo "  Executables:  $(TARGETS)"
	@echo ""
	@echo "🔧 Usage examples:"
	@echo "  make         - Build everything"
	@echo "  make js-gen  - Generate compilation database"
	@echo "  make watch   - Start development mode with auto-regeneration"
