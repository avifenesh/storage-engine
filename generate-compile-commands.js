#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

// ARM64/AArch64 Kernel-style compilation flags
const KERNEL_FLAGS = [
    // Core warning flags
    "-Wall",
    "-Wextra", 
    "-Werror",
    "-Wno-unused-parameter",
    "-Wno-sign-compare",
    "-Wno-unused-function",
    "-Wno-unused-variable",
    "-Wno-format-zero-length",
    "-Wdeclaration-after-statement",
    "-Wvla",
    
    // Standard and compatibility
    "-std=gnu11",
    "-fno-strict-aliasing",
    "-fno-common",
    "-fshort-wchar",
    "-fno-PIE",
    
    // ARM64-specific architecture flags
    "-march=armv8-a",
    "-mtune=generic",
    "-mgeneral-regs-only",
    "-DCONFIG_ARM64",
    "-DCONFIG_64BIT",
    
    // Memory model and alignment
    "-mcmodel=small",
    "-falign-functions=4",
    "-falign-jumps=1",
    "-falign-loops=1",
    
    // Security features
    "-fstack-protector-strong",
    "-fno-delete-null-pointer-checks",
    "-fno-allow-store-data-races",
    "-Wframe-larger-than=2048",
    
    // Code generation
    "-pipe",
    "-fno-asynchronous-unwind-tables",
    "-fno-jump-tables",
    "-fno-strict-overflow",
    "-fno-stack-check",
    "-fconserve-stack",
    
    // Warning suppressions for kernel-style code
    "-Wimplicit-fallthrough",
    "-Wno-unused-but-set-variable",
    "-Wno-unused-const-variable",
    "-Wno-pointer-sign",
    "-Wno-stringop-truncation",
    "-Wno-array-bounds",
    "-Wno-stringop-overflow",
    "-Wno-restrict",
    "-Wno-maybe-uninitialized",
    "-Wno-packed-not-aligned",
    
    // Optimization and debugging
    "-O2",
    "-fomit-frame-pointer",
    "-g",
    "-gdwarf-4",
    
    // Strict error checking
    "-Werror=date-time",
    "-Werror=incompatible-pointer-types",
    "-Werror=designated-init",
    "-Werror=implicit-function-declaration",
    
    // Build system integration
    "-fmacro-prefix-map=./=",
    
    // ARM64 kernel config defines
    "-DCONFIG_AS_CFI=1",
    "-DCONFIG_AS_CFI_SIGNAL_FRAME=1",
    "-DCONFIG_AS_CFI_SECTIONS=1"
];

function findCFiles(dir, fileList = []) {
    const files = fs.readdirSync(dir);
    
    files.forEach(file => {
        const filePath = path.join(dir, file);
        const stat = fs.statSync(filePath);
        
        if (stat.isDirectory()) {
            // Skip hidden directories like .git
            if (!file.startsWith('.')) {
                findCFiles(filePath, fileList);
            }
        } else if (file.endsWith('.c')) {
            fileList.push(filePath);
        }
    });
    
    return fileList;
}

function generateCompileCommands() {
    const projectDir = process.cwd();
    const outputFile = 'compile_commands.json';
    
    console.log(`🔍 Scanning for C files in ${projectDir}...`);
    
    const cFiles = findCFiles(projectDir);
    const commands = [];
    
    cFiles.forEach(filePath => {
        const relativePath = path.relative(projectDir, filePath);
        const absolutePath = path.resolve(filePath);
        const outputPath = relativePath.replace('.c', '.o');
        
        const command = {
            directory: projectDir,
            file: relativePath,
            output: outputPath,
            arguments: [
                "gcc",
                ...KERNEL_FLAGS,
                "-c",
                relativePath
            ],
            // Also include command field for better compatibility
            command: `gcc ${KERNEL_FLAGS.join(' ')} -c ${relativePath}`
        };
        
        commands.push(command);
    });
    
    // Write to file with proper formatting
    fs.writeFileSync(outputFile, JSON.stringify(commands, null, 2));
    
    console.log(`✅ Generated ${outputFile} with ${commands.length} C files`);
    console.log(`🔧 Using kernel-style compilation flags`);
    console.log(`📁 Files processed:`);
    commands.forEach(cmd => console.log(`   - ${cmd.file}`));
}

// Auto-regeneration watcher (optional)
function watchForChanges() {
    console.log(`👀 Watching for new .c files...`);
    
    fs.watch('.', { recursive: true }, (eventType, filename) => {
        if (filename && filename.endsWith('.c') && eventType === 'rename') {
            console.log(`📝 Detected change: ${filename}`);
            setTimeout(() => {
                try {
                    generateCompileCommands();
                } catch (error) {
                    console.error('Error regenerating compile commands:', error.message);
                }
            }, 100); // Small delay to ensure file operations complete
        }
    });
}

// Main execution
if (require.main === module) {
    const args = process.argv.slice(2);
    
    if (args.includes('--watch') || args.includes('-w')) {
        generateCompileCommands();
        watchForChanges();
    } else {
        generateCompileCommands();
    }
}
