# Sprint 2: Hash Table Storage Engine Makefile
# Builds kernel module and user-space tests for ARM64 hash storage engine

# Kernel module configuration
MODULE_NAME := hash_storage
KERNEL_OBJECTS := hash_engine.o hash_device.o neon_hash.o
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(KERNEL_OBJECTS)

# Kernel build directory - adjust for your system
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# ARM64 compilation flags for kernel module
EXTRA_CFLAGS += -DCONFIG_ARM64
EXTRA_CFLAGS += -march=armv8-a+simd
EXTRA_CFLAGS += -O2
EXTRA_CFLAGS += -Wall -Wextra
EXTRA_CFLAGS += -fno-strict-aliasing

# User-space test compilation
CC := gcc
CFLAGS := -Wall -Wextra -O2 -std=c99
CFLAGS += -march=armv8-a+simd
CFLAGS += -D_GNU_SOURCE

# Test programs
TEST_SOURCES := tests/hash_tests.c
TEST_PROGRAMS := $(TEST_SOURCES:.c=)

# Phony targets
.PHONY: all clean kernel tests install load unload test help

# Default target
all: kernel tests

# Build kernel module
kernel:
	@echo "Building hash storage kernel module..."
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

# Build user-space tests
tests: $(TEST_PROGRAMS)

tests/hash_tests: tests/hash_tests.c
	@echo "Building hash tests..."
	$(CC) $(CFLAGS) -o $@ $< -lrt

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f $(TEST_PROGRAMS)
	rm -f *.o *.ko *.mod.c *.mod *.order *.symvers
	rm -f .*.cmd .tmp_versions modules.order Module.symvers

# Install kernel module (requires root)
install: kernel
	@echo "Installing kernel module..."
	sudo cp $(MODULE_NAME).ko /lib/modules/$(shell uname -r)/extra/
	sudo depmod -a

# Load kernel module (requires root)
load: kernel
	@echo "Loading hash storage kernel module..."
	sudo insmod $(MODULE_NAME).ko
	@echo "Creating device node..."
	@sudo rm -f /dev/storage-hash
	@MAJOR=$$(awk '/storage-hash/ {print $$1}' /proc/devices); \
	if [ -n "$$MAJOR" ]; then \
		sudo mknod /dev/storage-hash c $$MAJOR 0; \
		sudo chmod 666 /dev/storage-hash; \
		echo "Device /dev/storage-hash created with major number $$MAJOR"; \
	else \
		echo "ERROR: Could not find major number for storage-hash"; \
		exit 1; \
	fi

# Unload kernel module (requires root)
unload:
	@echo "Unloading hash storage kernel module..."
	-sudo rmmod $(MODULE_NAME)
	-sudo rm -f /dev/storage-hash

# Run tests (requires loaded module)
test: tests
	@echo "Running hash storage tests..."
	@if [ ! -e /dev/storage-hash ]; then \
		echo "ERROR: /dev/storage-hash not found. Run 'make load' first."; \
		exit 1; \
	fi
	./tests/hash_tests

# Full test cycle: build, load, test, unload
test-full: all
	@echo "Running full test cycle..."
	$(MAKE) unload || true
	$(MAKE) load
	$(MAKE) test
	$(MAKE) unload

# Check kernel module info
info:
	@if lsmod | grep -q $(MODULE_NAME); then \
		echo "Module $(MODULE_NAME) is loaded:"; \
		lsmod | grep $(MODULE_NAME); \
		echo ""; \
		echo "Device information:"; \
		ls -l /dev/storage-hash 2>/dev/null || echo "/dev/storage-hash not found"; \
		echo ""; \
		echo "Kernel messages (last 10):"; \
		dmesg | grep "storage-hash" | tail -10; \
	else \
		echo "Module $(MODULE_NAME) is not loaded"; \
	fi

# Generate assembly files for analysis
asm: hash_engine.s hash_device.s neon_hash.s

%.s: %.c
	@echo "Generating assembly for $<..."
	$(CC) $(CFLAGS) -DCONFIG_ARM64 -I$(KERNEL_DIR)/include -S -o $@ $<

# Performance benchmarking
benchmark: test
	@echo "Running performance benchmark..."
	@echo "Testing with different data sizes..."
	@for size in 100 500 1000 2000; do \
		echo "Testing $$size operations:"; \
		timeout 30s ./tests/hash_tests || echo "Timeout or error with $$size operations"; \
		echo ""; \
	done

# Memory usage analysis with valgrind (user-space parts only)
memcheck: tests
	@echo "Running memory analysis on tests..."
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./tests/hash_tests

# Debug: show compilation commands
debug:
	@echo "Kernel build command:"
	@echo "$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules V=1"
	@echo ""
	@echo "Test build command:"
	@echo "$(CC) $(CFLAGS) -o tests/hash_tests tests/hash_tests.c -lrt"

# Help target
help:
	@echo "Hash Storage Engine Build System"
	@echo "================================="
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build kernel module and tests (default)"
	@echo "  kernel     - Build kernel module only"
	@echo "  tests      - Build user-space tests only"
	@echo "  clean      - Clean all build artifacts"
	@echo ""
	@echo "Module Management (requires root):"
	@echo "  load       - Load kernel module and create device node"
	@echo "  unload     - Unload kernel module and remove device node"
	@echo "  install    - Install module to system modules directory"
	@echo ""
	@echo "Testing:"
	@echo "  test       - Run tests (requires loaded module)"
	@echo "  test-full  - Complete test cycle (build + load + test + unload)"
	@echo "  benchmark  - Run performance benchmarks"
	@echo ""
	@echo "Analysis:"
	@echo "  info       - Show module status and recent kernel messages"
	@echo "  asm        - Generate assembly files for analysis"
	@echo "  memcheck   - Run valgrind memory analysis on tests"
	@echo ""
	@echo "Prerequisites:"
	@echo "  - ARM64 system with kernel headers installed"
	@echo "  - GCC with ARM NEON support"
	@echo "  - Root access for module loading"
	@echo ""
	@echo "Example usage:"
	@echo "  make all && sudo make load && make test"