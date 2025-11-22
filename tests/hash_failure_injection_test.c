/**
 * @file hash_failure_injection_test.c
 * @brief Failure injection testing for hash table
 *
 * Tests hash table behavior when various operations fail, including:
 * - Memory allocation failures
 * - Partial operation failures
 * - State corruption scenarios
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage/hash_engine.h"

#define TEST_PASSED 0
#define TEST_FAILED 1

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define RUN_TEST(test)                                                        \
	do {                                                                  \
		printf("Running %s...", #test);                               \
		fflush(stdout);                                               \
		tests_run++;                                                  \
		if ((test)() == TEST_PASSED) {                                \
			printf(" PASSED\n");                                  \
			tests_passed++;                                       \
		} else {                                                      \
			printf(" FAILED\n");                                  \
			tests_failed++;                                       \
		}                                                             \
	} while (0)

/*
 * NOTE: These tests simulate failure scenarios.
 * In a real implementation with failure injection hooks,
 * these would actually trigger failures in the allocation layer.
 */

/* Test: Graceful handling of near-OOM conditions */
static int
test_near_oom_behavior(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char *large_value;
	int rc;
	int i;
	int successful_inserts;
	const int ATTEMPTS = 100;
	const size_t LARGE_SIZE = 1024 * 1024; /* 1MB */

	rc = hash_engine_init(&engine, 128);
	if (rc != 0) {
		return TEST_FAILED;
	}

	successful_inserts = 0;

	/* Try to insert large values until we hit memory pressure */
	for (i = 0; i < ATTEMPTS; i++) {
		snprintf(key_buf, sizeof(key_buf), "oom_key_%d", i);

		large_value = malloc(LARGE_SIZE);
		if (!large_value) {
			/* We've hit actual memory limit */
			break;
		}

		memset(large_value, 'V', LARGE_SIZE);

		rc = hash_put(&engine, key_buf, strlen(key_buf), large_value,
			      LARGE_SIZE);

		if (rc == 0) {
			successful_inserts++;
		}

		free(large_value);

		/* Stop if we start failing */
		if (rc != 0 && i > 10) {
			break;
		}
	}

	printf("\n  Successfully inserted %d large values\n",
	       successful_inserts);

	/* Engine should still be functional */
	uint32_t item_count;
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0) {
		fprintf(stderr, "  Engine corrupted after memory pressure\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	printf("  Engine remains functional with %u items\n", item_count);

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Behavior with partial operation sequences */
static int
test_partial_operations(void)
{
	struct hash_engine engine;
	const char *key = "partial_key";
	const char *value1 = "value1";
	const char *value2 = "value2";
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;

	rc = hash_engine_init(&engine, 64);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert */
	rc = hash_put(&engine, key, strlen(key), value1, strlen(value1));
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Attempt to update (might fail) */
	rc = hash_put(&engine, key, strlen(key), value2, strlen(value2));
	/* We accept either success or failure here */

	/* Verify engine is still consistent */
	rc = hash_get(&engine, key, strlen(key), &retrieved_value,
		      &retrieved_len);
	if (rc != 0) {
		fprintf(stderr,
			"  Key disappeared after partial update attempt\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Value should be either value1 or value2, but not corrupted */
	if ((retrieved_len == strlen(value1)
	     && memcmp(retrieved_value, value1, strlen(value1)) == 0)
	    || (retrieved_len == strlen(value2)
		&& memcmp(retrieved_value, value2, strlen(value2)) == 0)) {
		printf("\n  Value is consistent after partial update\n");
	} else {
		fprintf(stderr, "  Value corrupted after partial update\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Handling of invalid state transitions */
static int
test_invalid_state_transitions(void)
{
	struct hash_engine engine;
	const char *key = "state_key";
	const char *value = "state_value";
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;

	rc = hash_engine_init(&engine, 64);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Try to delete non-existent key */
	rc = hash_delete(&engine, key, strlen(key));
	if (rc == 0) {
		fprintf(stderr, "  Delete of non-existent key succeeded\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Try to get non-existent key */
	rc = hash_get(&engine, key, strlen(key), &retrieved_value,
		      &retrieved_len);
	if (rc == 0) {
		fprintf(stderr, "  Get of non-existent key succeeded\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Insert key */
	rc = hash_put(&engine, key, strlen(key), value, strlen(value));
	if (rc != 0) {
		fprintf(stderr, "  Insert failed\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Delete key */
	rc = hash_delete(&engine, key, strlen(key));
	if (rc != 0) {
		fprintf(stderr, "  Delete failed\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Try to delete again */
	rc = hash_delete(&engine, key, strlen(key));
	if (rc == 0) {
		fprintf(stderr, "  Double delete succeeded\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	printf("\n  All invalid state transitions properly rejected\n");

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Recovery from resize failures */
static int
test_resize_failure_recovery(void)
{
	struct hash_engine engine;
	char key_buf[64];
	const char *value = "resize_value";
	uint32_t initial_bucket_count;
	uint32_t current_bucket_count;
	int rc;
	int i;
	int inserted_before_failure;

	rc = hash_engine_init(&engine, INITIAL_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	rc = hash_engine_get_stats(&engine, NULL, &initial_bucket_count, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Insert items until we would trigger a resize */
	int max_items = (int)(initial_bucket_count * MAX_LOAD_FACTOR) + 5;

	inserted_before_failure = 0;
	for (i = 0; i < max_items; i++) {
		snprintf(key_buf, sizeof(key_buf), "resize_fail_key_%d", i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value,
			      strlen(value));

		if (rc != 0) {
			/* Resize might have failed */
			printf("\n  Insert failed at item %d (possible resize "
			       "failure)\n",
			       i);
			break;
		}
		inserted_before_failure++;
	}

	/* Verify engine is still functional */
	rc = hash_engine_get_stats(&engine, NULL, &current_bucket_count, NULL);
	if (rc != 0) {
		fprintf(stderr, "  Engine corrupted after resize failure\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Try to perform more operations */
	rc = hash_get(&engine, "resize_fail_key_0",
		      strlen("resize_fail_key_0"), (const void **)&value,
		      (size_t *)&i);
	if (rc != 0) {
		fprintf(stderr, "  Cannot retrieve keys after resize "
				"failure\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	printf("  Engine recovered with %u buckets, %d items inserted\n",
	       current_bucket_count, inserted_before_failure);

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Handling of maximum capacity */
static int
test_max_capacity_handling(void)
{
	struct hash_engine engine;
	char key_buf[64];
	const char *value = "capacity_value";
	uint32_t bucket_count;
	int rc;
	int i;
	int max_attempts;

	/* Start with max bucket count */
	rc = hash_engine_init(&engine, MAX_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "  Cannot init with MAX_BUCKET_COUNT\n");
		return TEST_FAILED;
	}

	rc = hash_engine_get_stats(&engine, NULL, &bucket_count, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	printf("\n  Testing with maximum bucket count: %u\n", bucket_count);

	/* Try to fill beyond capacity */
	max_attempts = bucket_count + 1000;
	for (i = 0; i < max_attempts; i++) {
		snprintf(key_buf, sizeof(key_buf), "max_cap_key_%d", i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value,
			      strlen(value));

		if (rc != 0 && rc != -ENOSPC) {
			fprintf(stderr,
				"  Unexpected error at item %d: %d\n", i, rc);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		if (rc == -ENOSPC) {
			printf("  Properly returned ENOSPC at item %d\n", i);
			break;
		}
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

int
main(void)
{
	printf("===== Hash Table Failure Injection Tests =====\n");
	printf("Testing error handling and recovery\n\n");

	RUN_TEST(test_near_oom_behavior);
	RUN_TEST(test_partial_operations);
	RUN_TEST(test_invalid_state_transitions);
	RUN_TEST(test_resize_failure_recovery);
	RUN_TEST(test_max_capacity_handling);

	printf("\n========================================\n");
	printf("Tests run: %d\n", tests_run);
	printf("Tests passed: %d\n", tests_passed);
	printf("Tests failed: %d\n", tests_failed);
	printf("========================================\n");
	printf("\nNOTE: These tests verify error handling behavior.\n");
	printf("For actual malloc failure injection, use tools like:\n");
	printf("  - libfiu (Fault Injection in Userspace)\n");
	printf("  - failmalloc\n");
	printf("  - Custom malloc wrappers with LD_PRELOAD\n");

	return tests_failed > 0 ? 1 : 0;
}
