#include "disk_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple verifier that checks header footprint across a file of pages.
int
main(int argc, char **argv)
{
	const char *path;
	FILE *fp;
	unsigned char *buf;
	size_t page_no;
	int errors;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <datafile>\n", argv[0]);
		return 2;
	}
	path = argv[1];
	fp = fopen(path, "rb");
	if (!fp) {
		perror("fopen");
		return 1;
	}

	buf = (unsigned char *)malloc(PAGE_SIZE_BYTES);
	if (!buf) {
		fclose(fp);
		return 1;
	}
	page_no = 0;
	errors = 0;
	while (fread(buf, PAGE_SIZE_BYTES, 1, fp) == 1) {
		page_header_t hdr;
		page_read_header(buf, &hdr);
		if (hdr.page_type > PAGE_INDEX) {
			fprintf(stderr, "page %zu: invalid page_type=%u\n",
				page_no, hdr.page_type);
			errors++;
		}
		// Optionally verify checksum of payload here.
		page_no++;
	}
	free(buf);
	fclose(fp);
	if (errors) {
		fprintf(stderr, "fsck: %d error(s) found\n", errors);
		return 1;
	}
	printf("fsck: OK (%zu pages)\n", page_no);
	return 0;
}
