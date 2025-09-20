#ifndef SPRINT5_WAL_H
#define SPRINT5_WAL_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Minimal WAL record header.
typedef struct __attribute__((packed)) {
	uint32_t length; // payload length (bytes) following this header
	uint64_t lsn; // log sequence number
	uint32_t checksum; // CRC32C of payload (and header sans checksum)
} wal_record_hdr_t; // 16 bytes

typedef struct wal wal_t;

wal_t *wal_open(const char *path, size_t rotate_bytes);
void wal_close(wal_t *wal);

// Append a record payload; assigns LSN internally if lsn==0.
int wal_append(wal_t *wal, const void *payload, size_t len, uint64_t *out_lsn);
int wal_flush(wal_t *wal); // fsync/fflush depending on mode

// Replay records by invoking a callback for each valid record.
typedef int (*wal_apply_cb)(const wal_record_hdr_t *hdr, const void *data,
			    void *ctx);
int wal_replay(const char *path, wal_apply_cb apply, void *ctx);

#endif // SPRINT5_WAL_H
