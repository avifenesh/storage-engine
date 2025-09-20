#include "wal.h"
#include "disk_format.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct wal {
	FILE *fp;
	char *path;
	size_t size_bytes;
	size_t rotate_bytes;
	uint64_t next_lsn;
};

static int
wal_update_size(wal_t *wal)
{
	long pos;
	if (!wal || !wal->fp)
		return -EINVAL;
	if (fflush(wal->fp) != 0)
		return -errno;
	pos = ftell(wal->fp);
	if (pos < 0)
		return -errno;
	wal->size_bytes = (size_t)pos;
	return 0;
}

wal_t *
wal_open(const char *path, size_t rotate_bytes)
{
	wal_t *wal = (wal_t *)calloc(1, sizeof(*wal));
	if (!wal)
		return NULL;
	wal->path = strdup(path);
	wal->rotate_bytes = rotate_bytes ? rotate_bytes : (64 * 1024 * 1024);
	wal->next_lsn = 1;
	wal->fp = fopen(path, "ab+");
	if (!wal->fp) {
		free(wal->path);
		free(wal);
		return NULL;
	}
	fseek(wal->fp, 0, SEEK_END);
	wal_update_size(wal);
	return wal;
}

void
wal_close(wal_t *wal)
{
	if (!wal)
		return;
	if (wal->fp)
		fclose(wal->fp);
	free(wal->path);
	free(wal);
}

int
wal_append(wal_t *wal, const void *payload, size_t len, uint64_t *out_lsn)
{
	wal_record_hdr_t hdr;
	if (!wal || !wal->fp || (!payload && len))
		return -EINVAL;

	memset(&hdr, 0, sizeof(hdr));
	hdr.length = (uint32_t)len;
	hdr.lsn = wal->next_lsn++;

	// Compute checksum over payload only for this skeleton.
	hdr.checksum = crc32c(payload, len);

	if (fwrite(&hdr, sizeof(hdr), 1, wal->fp) != 1)
		return -errno;
	if (len && fwrite(payload, len, 1, wal->fp) != 1)
		return -errno;
	if (out_lsn)
		*out_lsn = hdr.lsn;

	wal_update_size(wal);
	return 0;
}

int
wal_flush(wal_t *wal)
{
	if (!wal || !wal->fp)
		return -EINVAL;
	if (fflush(wal->fp) != 0)
		return -errno;
	// fsync is platform specific; fflush is sufficient for this learning
	// skeleton.
	return 0;
}

int
wal_replay(const char *path, wal_apply_cb apply, void *ctx)
{
	FILE *fp;
	int rc;
	void *buf;
	uint32_t csum;
	if (!path || !apply)
		return -EINVAL;
	fp = fopen(path, "rb");
	if (!fp)
		return -errno;

	rc = 0;
	while (1) {
		wal_record_hdr_t hdr;
		size_t r = fread(&hdr, sizeof(hdr), 1, fp);
		if (r != 1) {
			rc = 0;
			break;
		} // End or partial header; treat as end

		if (hdr.length > (64 * 1024 * 1024)) {
			rc = -EINVAL;
			break;
		}
		buf = NULL;
		if (hdr.length) {
			buf = malloc(hdr.length);
			if (!buf) {
				rc = -ENOMEM;
				break;
			}
			if (fread(buf, hdr.length, 1, fp) != 1) {
				// Torn/partial record — stop replay without
				// error.
				free(buf);
				rc = 0;
				break;
			}
			csum = crc32c(buf, hdr.length);
			if (csum != hdr.checksum) {
				free(buf);
				rc = -EIO;
				break;
			}
		}

		rc = apply(&hdr, buf, ctx);
		free(buf);
		if (rc != 0)
			break;
	}
	fclose(fp);
	return rc;
}

// Simple CRC32C placeholder (NOT hardware-accelerated).
uint32_t
crc32c(const void *data, size_t len)
{
	const unsigned char *p = (const unsigned char *)data;
	uint32_t crc = 0xFFFFFFFFu;
	for (size_t i = 0; i < len; i++) {
		crc ^= p[i];
		for (int k = 0; k < 8; k++) {
			uint32_t mask = -(crc & 1);
			crc = (crc >> 1)
			      ^ (0x82F63B78u & mask); // Castagnoli polynomial
		}
	}
	return ~crc;
}
