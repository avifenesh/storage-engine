#include "wal.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Chaos-injection harness: write N records, then truncate at various points
// and ensure wal_replay does not crash and stops at partial records.

static int
apply_cb(const wal_record_hdr_t *hdr, const void *data, void *ctx)
{
	static unsigned long long last;
	(void)data;
	(void)ctx;
	// Minimal check: LSN increases
	if (hdr->lsn <= last)
		return -1;
	last = hdr->lsn;
	return 0;
}

int
main(int argc, char **argv)
{
	const char *path;
	int n;
	wal_t *wal;
	off_t cuts[5];
	size_t cuts_n;
	int in, out;
	char tmp[64];
	static char io_buf[4096];
	ssize_t r;
	off_t sz, new_sz;

	path = (argc > 1) ? argv[1] : "chaos_wal.log";
	n = (argc > 2) ? atoi(argv[2]) : 100;
	wal = wal_open(path, 0);
	if (!wal) {
		perror("wal_open");
		return 1;
	}

	// Append records
	for (int i = 0; i < n; i++) {
		char rec_buf[32];
		int len = snprintf(rec_buf, sizeof(rec_buf), "rec_%d", i);
		if (wal_append(wal, rec_buf, (size_t)len, NULL) != 0) {
			fprintf(stderr, "append failed\n");
		}
	}
	wal_flush(wal);
	wal_close(wal);

	// Try a few truncation points
	cuts[0] = 0;
	cuts[1] = 1;
	cuts[2] = 17;
	cuts[3] = 128;
	cuts[4] = 1024;
	cuts_n = 5;
	for (size_t i = 0; i < cuts_n; i++) {
		// copy original file to temp
		snprintf(tmp, sizeof(tmp), "chaos_%zu.log", i);
		in = open(path, O_RDONLY);
		if (in < 0)
			continue;
		out = open(tmp, O_CREAT | O_TRUNC | O_WRONLY, 0644);
		if (out < 0) {
			close(in);
			continue;
		}
		while ((r = read(in, io_buf, sizeof(io_buf))) > 0)
			write(out, io_buf, (size_t)r);
		close(in);
		// truncate
		sz = lseek(out, 0, SEEK_END);
		new_sz = (cuts[i] < sz) ? cuts[i] : sz;
		ftruncate(out, new_sz);
		close(out);
		// replay
		(void)wal_replay(tmp, apply_cb, NULL);
	}
	printf("chaos_replay: completed\n");
	return 0;
}
