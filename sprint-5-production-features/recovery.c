#include "wal.h"
#include "disk_format.h"
#include <stdio.h>

// Minimal recovery skeleton: scan WAL and print records.
// Extend to apply changes to pages via page cache and persist.

static int print_record(const wal_record_hdr_t *hdr, const void *data, void *ctx)
{
    (void)data; (void)ctx;
    printf("replay LSN=%llu len=%u\n", (unsigned long long)hdr->lsn, hdr->length);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <wal-file>\n", argv[0]);
        return 2;
    }
    return wal_replay(argv[1], print_record, NULL);
}

