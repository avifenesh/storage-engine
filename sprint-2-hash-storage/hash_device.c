#include <stddef.h>

// Userspace stub so it compiles in this repo.
// Kernel implementation should be guarded under __KERNEL__ in your module project.

int hash_device_init(void)
{
    return 0; // placeholder
}

void hash_device_exit(void)
{
}

