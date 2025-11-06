#include <stdio.h>
#include <string.h>

#include "../hash_engine.h"

int
test_basic_operations(void)
{
	struct hash_engine engine;
	const char *key = "hash-tests-key";
	const char *value = "hash-tests-value";
	const void *out_value = NULL;
	size_t out_value_len = 0;
	int rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);

	if (rc != 0) {
		fprintf(stderr, "hash_tests: init failed (%d)\n", rc);
		return rc;
	}

	rc = hash_put(&engine, key, strlen(key), value, strlen(value));
	if (rc != 0) {
		fprintf(stderr, "hash_tests: hash_put failed (%d)\n", rc);
		goto cleanup;
	}

	rc = hash_get(&engine, key, strlen(key), &out_value, &out_value_len);
	if (rc != 0 || out_value_len != strlen(value)
	    || memcmp(out_value, value, out_value_len) != 0) {
		fprintf(stderr, "hash_tests: hash_get failed (%d)\n", rc);
		rc = (rc != 0) ? rc : -1;
		goto cleanup;
	}

	rc = hash_delete(&engine, key, strlen(key));
	if (rc != 0) {
		fprintf(stderr, "hash_tests: hash_delete failed (%d)\n", rc);
		goto cleanup;
	}

	rc = 0;

cleanup:
	hash_engine_destroy(&engine);
	return rc;
}

int
main(void)
{
	return test_basic_operations();
}
