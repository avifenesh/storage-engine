#ifndef SPRINT5_DISK_FORMAT_H
#define SPRINT5_DISK_FORMAT_H

#include <stddef.h>
#include <stdint.h>

// On-disk page format (4KB default)
#ifndef PAGE_SIZE_BYTES
#define PAGE_SIZE_BYTES 4096
#endif

// Page types
typedef enum {
	PAGE_FREE = 0,
	PAGE_DATA = 1,
	PAGE_META = 2,
	PAGE_INDEX = 3
} page_type_t;

// Page header is stored at the start of each page.
// Keep structure POD and fixed-size; avoid padding by using exact types.
typedef struct __attribute__((packed)) {
	uint64_t page_id; // logical page number
	uint32_t page_type; // page_type_t
	uint64_t lsn; // last-updated LSN
	uint32_t
	    checksum; // CRC32C of payload (and optionally header sans checksum)
	uint32_t reserved; // align to 32 bytes total header size
} page_header_t; // 32 bytes

static inline size_t
page_header_size(void)
{
	return sizeof(page_header_t);
}
static inline size_t
page_payload_size(void)
{
	return PAGE_SIZE_BYTES - sizeof(page_header_t);
}

// CRC32C (Castagnoli) stub; implement in sprint as needed.
uint32_t crc32c(const void *data, size_t len);

// Serialize header into a 32-byte buffer (host-endian to little-endian if
// needed). For learning scope we assume little-endian host; extend as needed.
static inline void
page_write_header(void *page_buf, const page_header_t *hdr)
{
	*(page_header_t *)page_buf = *hdr;
}

static inline void
page_read_header(const void *page_buf, page_header_t *out)
{
	*out = *(const page_header_t *)page_buf;
}

#endif // SPRINT5_DISK_FORMAT_H
