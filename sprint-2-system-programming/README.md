# Sprint 2: System Programming - Practice Area

This directory is for **YOU** to implement the Sprint 2 tasks. 

## Your Tasks

### Issue #7: Build basic char device with miscdevice interface
**File to create**: `char_device.c`
- **DON'T LOOK AT SOLUTIONS** - implement it yourself!
- Reference: [docs/sprints/sprint-2-system-programming.md](../docs/sprints/sprint-2-system-programming.md)

### Issue #8: Implement IOCTL protocol
**File to create**: `vss_ioctl.h`
- Define your own IOCTL commands
- Create data structures for vector operations

### Issue #9: Build Rust user-space client
**File to create**: `vss_client.rs` (when ready)

### Issue #10: Build simple in-kernel vector store
**File to create**: `vector_store.c` (when ready)

## Learning Resources
- [LDD3 Character Drivers](https://lwn.net/Kernel/LDD3/)
- [Kernel IOCTL docs](https://docs.kernel.org/driver-api/ioctl.html)
- [Embetronicx IOCTL tutorial](https://embetronicx.com/tutorials/linux/device-drivers/ioctl-tutorial-in-linux/)

## Testing Your Code
```bash
# Test compilation (from project root)
make

# Run specific tests when you create executables
make executables
```

**Remember**: The goal is to LEARN by DOING, not by copying!
