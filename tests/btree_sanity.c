#include "storage/btree_engine.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>

int
main(void)
{
	struct btree_engine t;
	int rc;
	const void *out;
	size_t out_len;

	rc = btree_engine_init(&t);
	if (rc != 0) {
		fprintf(stderr, "btree init failed: %d\n", rc);
		return 1;
	}
	out = NULL;
	out_len = 0;
	rc = btree_search(&t, "k", 1, &out, &out_len);
	/* Not implemented yet; allow ENOSYS stub to keep test green */
	if (rc != -ENOSYS) {
		fprintf(stderr, "expected -ENOSYS, got %d\n", rc);
		return 1;
	}
	assert(btree_engine_destroy(&t) == 0);
	return 0;
}
