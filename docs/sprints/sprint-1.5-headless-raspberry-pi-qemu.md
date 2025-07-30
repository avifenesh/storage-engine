# Sprint 1.5: Headless Raspberry Pi Development Environment (QEMU)

## Overview
This sprint establishes a safe, headless development environment for kernel module testing using QEMU to emulate Raspberry Pi OS on your headless Debian ARM system. This provides an isolated environment for storage engine development before deploying to actual hardware.

## Tasks

### Issue #52: Complete QEMU Raspberry Pi Environment Setup ✅ COMPLETED
**GitHub Issue:** [Sprint 1.5: Headless Raspberry Pi Development Environment (QEMU)](https://github.com/avifenesh/storage-engine/issues/52)

**Learning Objectives:**
- Master headless-on-headless VM development workflow
- Set up QEMU Raspberry Pi OS emulation with SSH access
- Create reproducible development environment for kernel testing
- Learn ARM64-to-ARM emulation best practices
- Establish safe kernel module testing workflow

---

### Task 1: QEMU Installation and Setup 📋 PENDING
**Directory:** `setup/`
**Learning Objectives:**
- Install QEMU with ARM64 support
- Verify Raspberry Pi machine type availability
- Configure host system for VM development

**Installation Commands:**
```bash
# Install QEMU and required tools
sudo apt update
sudo apt install qemu-system-arm qemu-utils wget unzip

# Verify QEMU ARM support
qemu-system-aarch64 -machine help | grep raspi
```

**Expected Output:**
- `raspi3b` machine type available
- `raspi4b` machine type available (if QEMU 9.0+)

**Key Concepts:**
- QEMU machine types for Raspberry Pi emulation
- ARM64 host running ARM guest emulation
- Package dependencies for headless operation

**Resources:**
- [QEMU ARM Documentation](https://qemu.readthedocs.io/en/latest/system/target-arm.html)

---

### Task 2: Raspberry Pi OS Image Preparation 📋 PENDING
**Files:** `setup/prepare-image.sh`, prepared image file
**Learning Objectives:**
- Download and modify Raspberry Pi OS Lite image
- Configure headless boot with SSH access
- Set up user credentials for remote access

**Image Setup Process:**
```bash
# Download Raspberry Pi OS Lite (headless-friendly)
wget https://downloads.raspberrypi.org/raspios_lite_arm64/images/raspios_lite_arm64-2024-07-04/2024-07-04-raspios-bookworm-arm64-lite.img.xz
unxz 2024-07-04-raspios-bookworm-arm64-lite.img.xz

# Mount boot partition for headless configuration
sudo losetup -P /dev/loop0 2024-07-04-raspios-bookworm-arm64-lite.img
mkdir -p /tmp/rpi-boot
sudo mount /dev/loop0p1 /tmp/rpi-boot

# Enable SSH service
sudo touch /tmp/rpi-boot/ssh

# Create default user (replace credentials as needed)
echo 'pi:'$(echo 'your-secure-password' | openssl passwd -6 -stdin) | sudo tee /tmp/rpi-boot/userconf.txt

# Clean up
sudo umount /tmp/rpi-boot
sudo losetup -d /dev/loop0
```

**Key Concepts:**
- Headless Raspberry Pi OS configuration
- SSH enablement via `ssh` file
- User creation via `userconf.txt`
- Loop device mounting for image modification
- Password hashing with `openssl passwd -6`

**Resources:**
- [Raspberry Pi Headless Setup Guide](https://www.raspberrypi.org/documentation/configuration/headless.md)

---

### Task 3: QEMU VM Configuration Script 📋 PENDING
**File:** `setup/start-rpi-vm.sh`
**Learning Objectives:**
- Create reproducible VM startup script
- Configure networking for SSH access
- Optimize QEMU parameters for development

**Startup Script:**
```bash
#!/bin/bash
# Raspberry Pi QEMU Startup Script

IMAGE_FILE="2024-07-04-raspios-bookworm-arm64-lite.img"
SSH_PORT="2222"

echo "Starting Raspberry Pi VM (SSH: localhost:$SSH_PORT)"

qemu-system-aarch64 \
    -machine raspi3b \
    -m 1G \
    -drive file=$IMAGE_FILE,format=raw,if=sd \
    -netdev user,id=net0,hostfwd=tcp::$SSH_PORT-:22 \
    -device usb-net,netdev=net0 \
    -nographic \
    -serial stdio

echo "VM shutdown complete"
```

**Advanced Configuration (Raspberry Pi 4):**
```bash
# Alternative configuration for raspi4b (requires QEMU 9.0+)
qemu-system-aarch64 \
    -machine raspi4b \
    -m 4G \
    -drive file=$IMAGE_FILE,format=raw,if=sd \
    -netdev user,id=net0,hostfwd=tcp::$SSH_PORT-:22 \
    -device usb-net,netdev=net0 \
    -nographic
```

**Key Concepts:**
- QEMU machine selection: `raspi3b` vs `raspi4b`
- Memory allocation: 1GB for raspi3b, 4GB for raspi4b
- Network configuration: user-mode networking with port forwarding
- Headless operation: `-nographic`, `-serial stdio`
- Storage: SD card emulation with raw image format

**Resources:**
- [QEMU Networking Documentation](https://wiki.qemu.org/Documentation/Networking)

---

### Task 4: SSH Access and Development Workflow 📋 PENDING
**Learning Objectives:**
- Establish reliable SSH connection to VM
- Set up development environment inside VM
- Test kernel module compilation workflow

**Connection Process:**
```bash
# Wait for VM to boot (2-3 minutes)
# Connect via SSH
ssh pi@localhost -p 2222

# First login tasks
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)

# Test kernel module compilation
echo 'obj-m += hello.o' > Makefile
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
```

**Development Workflow:**
1. **Code on Host**: Edit files in your main development environment
2. **Transfer to VM**: Use `scp -P 2222` or shared directories
3. **Test in VM**: Compile and load kernel modules safely
4. **Debug**: Use VM's isolation for crash recovery

**File Transfer Examples:**
```bash
# Host to VM
scp -P 2222 mymodule.c pi@localhost:~/

# VM to Host
scp -P 2222 pi@localhost:~/results.txt ./
```

**Key Concepts:**
- SSH port forwarding: host port 2222 → guest port 22
- Cross-system file transfer with `scp`
- Kernel headers installation for module compilation
- Isolated testing environment benefits

**Resources:**
- [SSH Port Forwarding Guide](https://www.ssh.com/academy/ssh/tunneling/example)

---

### Task 5: Storage Engine Testing Preparation 📋 PENDING
**File:** `setup/test-kernel-module.sh`
**Learning Objectives:**
- Verify kernel module development environment
- Test basic device creation and IOCTL operations
- Establish baseline for storage engine development

**Test Storage Engine Basics:**
```bash
# In the VM, test device creation
sudo mknod /dev/test-storage c 100 0
ls -la /dev/test-storage

# Test kernel module basics
cat > hello_module.c << 'EOF'
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init hello_init(void) {
    printk(KERN_INFO "Hello from storage engine test!\n");
    return 0;
}

static void __exit hello_exit(void) {
    printk(KERN_INFO "Goodbye from storage engine test!\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Storage Engine Test Module");
EOF

# Test compilation and loading
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
sudo insmod hello_module.ko
dmesg | tail -1
sudo rmmod hello_module
dmesg | tail -1
```

**Validation Checklist:**
- [ ] Module compiles without errors
- [ ] Module loads successfully (`insmod`)
- [ ] Module appears in `lsmod` output
- [ ] Module logs appear in `dmesg`
- [ ] Module unloads cleanly (`rmmod`)
- [ ] No error messages in kernel logs

**Key Concepts:**
- Character device node creation with `mknod`
- Kernel module structure: `module_init`, `module_exit`
- Kernel logging with `printk`
- Module metadata: `MODULE_LICENSE`, `MODULE_DESCRIPTION`
- Safe module loading/unloading cycle

**Resources:**
- [Linux Kernel Module Programming Guide](https://tldp.org/LDP/lkmpg/2.6/html/index.html)

---

### Task 6: Environment Documentation and Automation 📋 PENDING
**Files:** `setup/README.md`, `setup/vm-management.sh`
**Learning Objectives:**
- Document complete setup process
- Create automation scripts for VM lifecycle
- Establish reproducible environment

**VM Management Script:**
```bash
#!/bin/bash
# VM Management Script

case "$1" in
    start)
        ./start-rpi-vm.sh
        ;;
    ssh)
        ssh pi@localhost -p 2222
        ;;
    copy-to-vm)
        scp -P 2222 "$2" pi@localhost:~/
        ;;
    copy-from-vm)
        scp -P 2222 pi@localhost:"$2" ./
        ;;
    status)
        if pgrep -f "qemu-system-aarch64.*raspi" > /dev/null; then
            echo "VM is running"
        else
            echo "VM is not running"
        fi
        ;;
    *)
        echo "Usage: $0 {start|ssh|copy-to-vm|copy-from-vm|status}"
        exit 1
        ;;
esac
```

**Documentation Topics:**
- Complete setup walkthrough
- Troubleshooting common issues
- Performance optimization tips
- Security considerations for development

**Key Concepts:**
- Shell script automation
- VM lifecycle management
- Documentation as code
- Reproducible development environments

**Resources:**
- [Shell Scripting Guide](https://www.shellscript.sh/)

## Networking Architecture

```
┌─────────────────────────────────────┐
│ Headless Debian ARM Host            │
│                                     │
│ ┌─────────────────────────────────┐ │
│ │ QEMU Raspberry Pi VM            │ │
│ │                                 │ │
│ │ Raspberry Pi OS (ARM64)         │ │
│ │ - SSH Server (port 22)          │ │
│ │ - Kernel Development Tools      │ │
│ │ - Storage Engine Testing        │ │
│ │                                 │ │
│ └─────────────────────────────────┘ │
│                                     │
│ Host Network Interface              │
│ - SSH Client                        │
│ - Development Tools                 │
│ - File Editor                       │
└─────────────────────────────────────┘
         │
         │ SSH Connection
         │ localhost:2222 → VM:22
         │
    ┌─────────┐
    │   You   │
    │  (SSH)  │
    └─────────┘
```

## Development Workflow

1. **Edit Code**: On your main development system
2. **Transfer Files**: `scp -P 2222 file.c pi@localhost:~/`
3. **Compile & Test**: Inside the VM via SSH
4. **Debug**: Use VM isolation for safe kernel experimentation
5. **Deploy**: Eventually to real Raspberry Pi hardware

## Sprint Goals
- Establish headless Raspberry Pi development environment
- Master QEMU VM management for kernel development
- Create reproducible testing environment
- Prepare foundation for storage engine implementation
- Document complete setup for team collaboration

## Success Criteria
- [ ] QEMU VM boots Raspberry Pi OS successfully
- [ ] SSH connection works reliably from host
- [ ] Can compile and load kernel modules in VM
- [ ] VM provides isolated testing environment
- [ ] Setup is fully documented and reproducible
- [ ] Environment ready for Sprint 2A development

## Next Sprint Preview
Sprint 2A will focus on user-space hash table implementation, building the algorithmic foundation before moving to kernel space in subsequent sprints. The QEMU environment established here will be used for safe kernel module testing throughout the project.

## Performance Expectations
- **VM Boot Time**: 2-3 minutes to SSH-ready state
- **SSH Latency**: <10ms for local connections
- **File Transfer**: ~10MB/s via SCP
- **Compilation**: Similar to native ARM64 performance
- **Memory Usage**: 1GB RAM for raspi3b, 4GB for raspi4b

## Security Considerations
- VM isolation prevents host system corruption
- SSH key-based authentication recommended
- Firewall rules for port 2222 access
- Regular VM snapshot backups
- Separate development credentials from production