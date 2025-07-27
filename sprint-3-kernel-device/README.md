# Sprint 3: Kernel Character Device

## Overview
This sprint ports the text search engine into Linux kernel space by creating a character device with IOCTL interface.

## Goals
- Create `/dev/textsearch` character device
- Implement IOCTL interface for search operations
- Port inverted index to kernel memory
- Handle concurrent access safely

## Key Files
- `textsearch_dev.c` - Main kernel module
- `textsearch_ioctl.h` - IOCTL command definitions
- `kernel_index.c` - In-kernel inverted index
- `kernel_search.c` - Search implementation
- `Makefile` - Kernel module build configuration

## Getting Started
1. Read the sprint documentation: `../docs/sprints/sprint-3-kernel-device.md`
2. Set up kernel development environment
3. Start with Issue #31 (basic character device)
4. Test in a VM first!

## Building and Loading
```bash
# Build kernel module
make -C /lib/modules/$(uname -r)/build M=$PWD modules

# Load module
sudo insmod textsearch.ko

# Check device
ls -l /dev/textsearch

# View kernel logs
sudo dmesg -w

# Unload module
sudo rmmod textsearch
```

## Testing
```bash
# Compile test program
gcc -o test_textsearch test_textsearch.c

# Run tests
sudo ./test_textsearch

# Check for memory leaks
sudo sh -c 'echo scan > /sys/kernel/debug/kmemleak'
sudo cat /sys/kernel/debug/kmemleak
```

## IOCTL Commands
- `TEXTSEARCH_ADD_DOC` - Add document to index
- `TEXTSEARCH_SEARCH` - Search for documents
- `TEXTSEARCH_GET_STATS` - Get index statistics
- `TEXTSEARCH_CLEAR_INDEX` - Clear the index

## Safety Notes
⚠️ **WARNING**: Kernel programming can crash your system!
- Always test in a VM first
- Use proper error handling
- Validate all user input
- Never dereference user pointers directly

## Resources
- [Linux Device Drivers, 3rd Edition](https://lwn.net/Kernel/LDD3/)
- [Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)
- [IOCTL Tutorial](https://embetronicx.com/tutorials/linux/device-drivers/ioctl-tutorial-in-linux/)