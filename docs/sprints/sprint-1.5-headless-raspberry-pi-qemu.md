# Sprint 1.5: EC2 Cross-Compilation & Raspberry Pi Hardware Development Environment

## Overview
This sprint establishes a professional kernel development workflow using Docker cross-compilation on EC2 and deployment to actual Raspberry Pi hardware. This provides authentic testing on real ARM hardware while maintaining a clean development environment separation.

## Architecture

```
Internet
    │
┌───┼─────────────────────────────────────┐
│   │ EC2 Instance (Debian ARM64 DevDesk) │
│   │ - Development & code editing        │
│   │ - Docker cross-compilation          │
│   │ - Git version control               │
│   │                                     │
│   │ ┌─────────────────────────────────┐ │
│   │ │ Docker Cross-Compile Container  │ │
│   │ │ - Raspberry Pi build env        │ │
│   │ │ - Kernel headers matching Pi    │ │
│   │ │ - ARM64 optimized builds        │ │
│   │ └─────────────────────────────────┘ │
│   └──────────────┬──────────────────────┘
│                  │ SSH Tunnel/VPN
│                  │ Port 2222 → Pi:22
│                  ▼
│           ┌─────────────────┐
│           │ Local Network   │
│           │ ┌─────────────┐ │
│           │ │ Raspberry Pi│ │
│           │ │ 192.168.1.x │ │
│           │ │ Real HW     │ │
│           │ └─────────────┘ │
│           └─────────────────┘
└─────────────────────────────────────────
```

## Tasks

### Issue #52: Complete EC2 + Raspberry Pi Development Setup ✅ COMPLETED
**GitHub Issue:** [Sprint 1.5: EC2 Cross-Compilation & Raspberry Pi Hardware Development Environment](https://github.com/avifenesh/storage-engine/issues/52)

**Learning Objectives:**
- Master Docker cross-compilation for Raspberry Pi
- Establish network connectivity from EC2 to local Raspberry Pi
- Create automated deployment pipeline
- Test on real ARM hardware for authentic performance
- Build professional kernel development workflow

---

### Task 1: Docker Cross-Compilation Environment 📋 PENDING
**Directory:** `setup/docker/`
**Learning Objectives:**
- Create Raspberry Pi cross-compilation container
- Set up proper kernel headers and build tools
- Establish consistent build environment

**Dockerfile** (`setup/docker/rpi-cross-compile/Dockerfile`):
```dockerfile
FROM arm64v8/debian:bookworm

# Install cross-compilation tools
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    wget \
    bc \
    bison \
    flex \
    libssl-dev \
    libelf-dev \
    rsync \
    ssh \
    curl

# Download Raspberry Pi kernel headers
RUN git clone --depth=1 --branch=rpi-6.6.y \
    https://github.com/raspberrypi/linux.git /usr/src/linux-rpi

# Configure kernel headers
WORKDIR /usr/src/linux-rpi
RUN make ARCH=arm64 bcm2711_defconfig
RUN make ARCH=arm64 modules_prepare

# Set up build environment
WORKDIR /workspace
ENV KERNEL_DIR=/usr/src/linux-rpi
ENV ARCH=arm64
ENV CROSS_COMPILE=aarch64-linux-gnu-

# Create build helper script
RUN echo '#!/bin/bash\nmake -C $KERNEL_DIR M=/workspace modules "$@"' > /usr/local/bin/build-modules
RUN chmod +x /usr/local/bin/build-modules

CMD ["/bin/bash"]
```

**Build Script** (`setup/docker/build-container.sh`):
```bash
#!/bin/bash
cd setup/docker/rpi-cross-compile
docker build -t rpi-cross-compile:latest .
echo "Cross-compilation container ready!"
```

**Key Concepts:**
- ARM64 native compilation (not cross-arch)
- Raspberry Pi specific kernel headers
- Consistent build environment isolation
- Automated kernel module compilation

**Resources:**
- [Raspberry Pi Kernel Building](https://www.raspberrypi.org/documentation/computers/linux_kernel.html)

---

### Task 2: Network Connectivity Solutions 📋 PENDING
**Files:** `setup/network/setup-tunnel.sh`
**Learning Objectives:**
- Establish secure connection from EC2 to local Raspberry Pi
- Configure reverse SSH tunnel or VPN
- Automate connection management

**Option 1 - Reverse SSH Tunnel (Recommended):**

**On Raspberry Pi** (`setup/network/pi-tunnel-client.sh`):
```bash
#!/bin/bash
# Raspberry Pi - Create reverse tunnel to EC2

EC2_HOST="your-ec2-public-ip"
EC2_USER="admin"
LOCAL_SSH_PORT="22"
REMOTE_TUNNEL_PORT="2222"

# Install autossh for persistent tunnels
sudo apt update
sudo apt install autossh

# Create reverse tunnel
autossh -M 20000 -N -R ${REMOTE_TUNNEL_PORT}:localhost:${LOCAL_SSH_PORT} \
    ${EC2_USER}@${EC2_HOST}
```

**Systemd Service for Persistent Tunnel** (`/etc/systemd/system/ec2-tunnel.service`):
```ini
[Unit]
Description=Reverse SSH Tunnel to EC2
After=network.target

[Service]
Type=simple
User=pi
ExecStart=/usr/bin/autossh -M 20000 -N -R 2222:localhost:22 admin@YOUR-EC2-PUBLIC-IP
Restart=always
RestartSec=10
Environment="AUTOSSH_GATETIME=0"

[Install]
WantedBy=multi-user.target
```

**On EC2** (`setup/network/test-connection.sh`):
```bash
#!/bin/bash
# Test connection to Raspberry Pi through tunnel

echo "Testing connection to Raspberry Pi..."
ssh -p 2222 -o ConnectTimeout=10 pi@localhost "hostname && uname -a"

if [ $? -eq 0 ]; then
    echo "✅ Connection successful!"
else
    echo "❌ Connection failed - check tunnel setup"
fi
```

**Option 2 - VPN Solution:**
```bash
# If you have VPN access to local network
# Configure WireGuard or OpenVPN client on EC2
# Then direct SSH: ssh pi@192.168.1.100
```

**Key Concepts:**
- Reverse SSH tunneling for NAT traversal
- Persistent connection with autossh
- Systemd service management
- Connection testing and monitoring

**Resources:**
- [SSH Tunneling Guide](https://www.ssh.com/academy/ssh/tunneling)
- [Autossh Documentation](https://linux.die.net/man/1/autossh)

---

### Task 3: Raspberry Pi Hardware Setup 📋 PENDING
**Files:** `setup/pi/configure-dev-environment.sh`
**Learning Objectives:**
- Configure Raspberry Pi for kernel development
- Install required packages and headers
- Set up development directories

**Raspberry Pi Setup Script** (`setup/pi/setup-pi.sh`):
```bash
#!/bin/bash
# Run this on the Raspberry Pi

echo "Setting up Raspberry Pi for kernel development..."

# Update system
sudo apt update && sudo apt upgrade -y

# Install kernel development packages
sudo apt install -y \
    raspberrypi-kernel-headers \
    build-essential \
    git \
    linux-headers-$(uname -r) \
    dkms \
    tree \
    htop

# Create development directories
mkdir -p ~/development/{modules,logs,backups}
mkdir -p ~/development/modules/{hash-storage,btree-storage,lsm-storage}

# Test kernel module compilation
cat > ~/development/test_module.c << 'EOF'
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init test_init(void) {
    printk(KERN_INFO "Raspberry Pi kernel development environment ready!\n");
    return 0;
}

static void __exit test_exit(void) {
    printk(KERN_INFO "Test module removed\n");
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Development Environment Test");
MODULE_VERSION("1.0");
EOF

# Create Makefile
echo "obj-m += test_module.o" > ~/development/Makefile

# Test compilation
cd ~/development
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules

if [ -f test_module.ko ]; then
    echo "✅ Kernel module compilation successful!"
    # Test loading
    sudo insmod test_module.ko
    dmesg | tail -1
    sudo rmmod test_module
    echo "✅ Module loading/unloading successful!"
else
    echo "❌ Kernel module compilation failed"
fi

echo "Raspberry Pi development environment ready!"
```

**Key Concepts:**
- Raspberry Pi specific kernel headers
- Development directory structure
- Kernel module compilation testing
- Hardware-specific optimizations

**Resources:**
- [Raspberry Pi OS Documentation](https://www.raspberrypi.org/documentation/)

---

### Task 4: Automated Deployment Pipeline 📋 PENDING
**Files:** `setup/deploy/deploy-to-pi.sh`, `Makefile` updates
**Learning Objectives:**
- Create automated build and deployment workflow
- Implement error handling and logging
- Establish testing automation

**Deployment Script** (`setup/deploy/deploy-to-pi.sh`):
```bash
#!/bin/bash
# Automated deployment from EC2 to Raspberry Pi

set -e  # Exit on any error

PI_CONNECTION="pi@localhost"
PI_PORT="2222"
MODULE_DIR="~/development/modules"
LOG_FILE="deployment.log"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log() {
    echo -e "${GREEN}[$(date '+%Y-%m-%d %H:%M:%S')]${NC} $1" | tee -a $LOG_FILE
}

error() {
    echo -e "${RED}[ERROR]${NC} $1" | tee -a $LOG_FILE
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $1" | tee -a $LOG_FILE
}

# Function to check Pi connection
check_pi_connection() {
    log "Checking Raspberry Pi connection..."
    if ssh -p $PI_PORT -o ConnectTimeout=10 $PI_CONNECTION "echo 'Connected'" > /dev/null 2>&1; then
        log "✅ Raspberry Pi connection successful"
        return 0
    else
        error "❌ Cannot connect to Raspberry Pi"
        return 1
    fi
}

# Function to build modules in Docker
build_modules() {
    log "Building modules in Docker container..."
    
    docker run --rm -v $(pwd):/workspace rpi-cross-compile:latest build-modules
    
    if [ $? -eq 0 ]; then
        log "✅ Module compilation successful"
    else
        error "❌ Module compilation failed"
        return 1
    fi
}

# Function to deploy modules
deploy_modules() {
    log "Deploying modules to Raspberry Pi..."
    
    # Copy modules to Pi
    scp -P $PI_PORT *.ko $PI_CONNECTION:$MODULE_DIR/
    
    # Copy test scripts
    scp -P $PI_PORT setup/deploy/test-modules.sh $PI_CONNECTION:$MODULE_DIR/
    
    log "✅ Modules deployed successfully"
}

# Function to run tests on Pi
run_tests() {
    log "Running tests on Raspberry Pi..."
    
    ssh -p $PI_PORT $PI_CONNECTION "cd $MODULE_DIR && ./test-modules.sh"
    
    if [ $? -eq 0 ]; then
        log "✅ All tests passed"
    else
        warn "⚠️  Some tests failed - check logs"
    fi
}

# Main deployment flow
main() {
    log "Starting deployment pipeline..."
    
    check_pi_connection || exit 1
    build_modules || exit 1
    deploy_modules || exit 1
    run_tests
    
    log "🎉 Deployment pipeline completed!"
}

# Run main function
main "$@"
```

**Test Script for Pi** (`setup/deploy/test-modules.sh`):
```bash
#!/bin/bash
# Run this on Raspberry Pi to test deployed modules

echo "Testing deployed kernel modules..."

for module in *.ko; do
    if [ -f "$module" ]; then
        echo "Testing module: $module"
        
        # Load module
        sudo insmod "$module"
        
        # Check if loaded
        if lsmod | grep -q "${module%.ko}"; then
            echo "✅ $module loaded successfully"
            
            # Check dmesg for messages
            dmesg | tail -5
            
            # Unload module
            sudo rmmod "${module%.ko}"
            echo "✅ $module unloaded successfully"
        else
            echo "❌ $module failed to load"
        fi
        
        echo "---"
    fi
done
```

**Makefile Integration**:
```makefile
# Add these targets to main Makefile

.PHONY: docker-build deploy test-pi clean-pi

docker-build:
	@echo "Building modules in Docker..."
	docker run --rm -v $(shell pwd):/workspace rpi-cross-compile:latest build-modules

deploy: docker-build
	@echo "Deploying to Raspberry Pi..."
	./setup/deploy/deploy-to-pi.sh

test-pi:
	@echo "Running tests on Raspberry Pi..."
	ssh -p 2222 pi@localhost "cd ~/development/modules && ./test-modules.sh"

clean-pi:
	@echo "Cleaning modules on Raspberry Pi..."
	ssh -p 2222 pi@localhost "cd ~/development/modules && rm -f *.ko && sudo dmesg -c > /dev/null"
```

**Key Concepts:**
- Automated CI/CD pipeline
- Error handling and logging
- Remote testing automation
- Build verification

**Resources:**
- [Bash Scripting Best Practices](https://bertvv.github.io/cheat-sheets/Bash.html)

---

### Task 5: Performance Benchmarking Setup 📋 PENDING
**Files:** `setup/benchmarks/pi-performance.sh`
**Learning Objectives:**
- Establish baseline performance metrics
- Create automated benchmark suite
- Compare development vs production performance

**Benchmark Script** (`setup/benchmarks/pi-performance.sh`):
```bash
#!/bin/bash
# Raspberry Pi performance benchmarking

echo "Raspberry Pi System Information:"
echo "================================"
cat /proc/cpuinfo | grep -E "model name|cpu MHz|cache size" | head -10
echo ""
free -h
echo ""
df -h
echo ""

echo "Storage Engine Performance Baseline:"
echo "===================================="

# Memory bandwidth test
echo "Memory bandwidth test..."
dd if=/dev/zero of=/tmp/test bs=1M count=100 2>&1 | grep copied

# CPU performance
echo "CPU performance test..."
time pi=$(echo "scale=1000; 4*a(1)" | bc -l)

# I/O performance  
echo "I/O performance test..."
sync && echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null
time dd if=/dev/zero of=/tmp/iotest bs=4k count=10000 2>/dev/null
rm -f /tmp/iotest

echo "Baseline tests completed!"
```

**Key Concepts:**
- Hardware performance characterization
- Baseline establishment for optimization
- Real hardware vs emulation comparison

---

### Task 6: Documentation and Automation 📋 PENDING
**Files:** `setup/README.md`, `setup/quick-start.sh`
**Learning Objectives:**
- Document complete workflow
- Create one-command setup
- Establish troubleshooting guides

**Quick Start Script** (`setup/quick-start.sh`):
```bash
#!/bin/bash
# One-command setup for EC2 + Raspberry Pi development

echo "🚀 Setting up EC2 + Raspberry Pi development environment..."

# Build Docker container
echo "📦 Building cross-compilation container..."
./setup/docker/build-container.sh

# Test Pi connection
echo "🔗 Testing Raspberry Pi connection..."
./setup/network/test-connection.sh

# Deploy test module
echo "🧪 Deploying test module..."
make deploy

echo "✅ Development environment ready!"
echo ""
echo "Quick commands:"
echo "  make docker-build  # Build modules"
echo "  make deploy        # Deploy to Pi"
echo "  make test-pi       # Run tests"
echo "  make clean-pi      # Clean Pi modules"
```

## Development Workflow

1. **Code Development**: Edit on EC2 using your preferred editor
2. **Cross Compilation**: `make docker-build` - Build in Docker container
3. **Deployment**: `make deploy` - Transfer to Raspberry Pi via SSH tunnel
4. **Testing**: `make test-pi` - Run automated tests on real hardware
5. **Debugging**: SSH to Pi for interactive debugging and profiling

## Sprint Goals
- Establish EC2 to Raspberry Pi development pipeline
- Master Docker cross-compilation for ARM kernel modules
- Create automated deployment and testing workflow
- Verify real hardware performance and behavior
- Document reproducible professional development setup

## Success Criteria
- [ ] Docker cross-compilation container builds successfully
- [ ] Network connection from EC2 to Raspberry Pi established
- [ ] Automated deployment pipeline functional
- [ ] Kernel modules compile and load on real Raspberry Pi
- [ ] Performance benchmarking baseline established
- [ ] Complete documentation and automation scripts

## Network Topology Summary

**EC2 Instance (Public)** ← Internet → **Your Location** → **Raspberry Pi (Private)**

Connection via:
- **Reverse SSH Tunnel**: Pi connects out to EC2, creates tunnel for EC2 to reach Pi
- **Port Forward**: EC2:2222 → Pi:22
- **Authentication**: SSH keys for security

## Performance Expectations
- **Cross-compilation**: 2-5x faster than Pi native compilation
- **Deployment**: ~30 seconds for typical module
- **SSH Latency**: <100ms via tunnel (depends on internet)
- **Real Hardware**: Authentic ARM64 performance characteristics
- **Development Cycle**: Code → Build → Deploy → Test in <2 minutes

## Security Considerations
- SSH key-based authentication only
- EC2 security groups limiting access
- Raspberry Pi firewall configuration
- Encrypted tunnel for all communications
- Separate development and production credentials

## Next Sprint Preview
Sprint 2A will focus on user-space hash table implementation using this established development environment. The Docker cross-compilation and Raspberry Pi deployment pipeline will accelerate the development-test cycle for all subsequent storage engine sprints.