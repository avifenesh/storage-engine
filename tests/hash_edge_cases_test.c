/**
 * @file hash_edge_cases_test.c
 * @brief Edge case and boundary condition tests for hash table
 *
 * Tests unusual inputs, boundary conditions, and corner cases that might
 * expose bugs in the implementation.
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage/hash_engine.h"

#define TEST_PASSED 0
#define TEST_FAILED 1

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define RUN_TEST(test)                                                         \
	do {                                                                   \
		printf("Running %s...", #test);                                \
		fflush(stdout);                                                \
		tests_run++;                                                   \
		if ((test)() == TEST_PASSED) {                                 \
			printf(" PASSED\n");                                   \
			tests_passed++;                                        \
		} else {                                                       \
			printf(" FAILED\n");                                   \
			tests_failed++;                                        \
		}                                                              \
	} while (0)

/* Test: NULL pointer handling */
static int
test_null_pointers(void)
{
	struct hash_engine engine;
	const char *key = "key";
	const char *value = "value";
	const void *out_value;
	size_t out_len;
	int rc;

	/* NULL engine to init */
	rc = hash_engine_init(NULL, DEFAULT_BUCKET_COUNT);
	if (rc != -EINVAL) {
		fprintf(stderr, "Init should reject NULL engine\n");
		return TEST_FAILED;
	}

	/* Valid init for remaining tests */
	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* NULL key to put */
	rc = hash_put(&engine, NULL, strlen(key), value, strlen(value));
	if (rc != -EINVAL) {
		fprintf(stderr, "Put should reject NULL key\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* NULL value to put */
	rc = hash_put(&engine, key, strlen(key), NULL, strlen(value));
	if (rc != -EINVAL) {
		fprintf(stderr, "Put should reject NULL value\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* NULL key to get */
	rc = hash_get(&engine, NULL, strlen(key), &out_value, &out_len);
	if (rc != -EINVAL) {
		fprintf(stderr, "Get should reject NULL key\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* NULL key to delete */
	rc = hash_delete(&engine, NULL, strlen(key));
	if (rc != -EINVAL) {
		fprintf(stderr, "Delete should reject NULL key\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* NULL engine to destroy */
	rc = hash_engine_destroy(NULL);
	if (rc != -EINVAL) {
		fprintf(stderr, "Destroy should reject NULL engine\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Zero-length key handling */
static int
test_zero_length_key(void)
{
	struct hash_engine engine;
	const char *key = "non_empty";
	const void *out_value;
	size_t out_len;
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Zero-length key to put */
	rc = hash_put(&engine, key, 0, "value", 5);
	if (rc != -EINVAL) {
		fprintf(stderr, "Put should reject zero-length key\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Zero-length key to get */
	rc = hash_get(&engine, key, 0, &out_value, &out_len);
	if (rc != -EINVAL) {
		fprintf(stderr, "Get should reject zero-length key\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Zero-length key to delete */
	rc = hash_delete(&engine, key, 0);
	if (rc != -EINVAL) {
		fprintf(stderr, "Delete should reject zero-length key\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Zero-length value handling */
static int
test_zero_length_value(void)
{
	struct hash_engine engine;
	const char *key = "key";
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Zero-length value to put */
	rc = hash_put(&engine, key, strlen(key), "value", 0);
	if (rc != -EINVAL) {
		fprintf(stderr, "Put should reject zero-length value\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Invalid bucket counts */
static int
test_invalid_bucket_counts(void)
{
	struct hash_engine engine;
	int rc;

	/* Zero bucket count */
	rc = hash_engine_init(&engine, 0);
	if (rc != -EINVAL) {
		fprintf(stderr, "Init should reject zero bucket count\n");
		if (rc == 0)
			hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Negative bucket count */
	rc = hash_engine_init(&engine, -100);
	if (rc != -EINVAL) {
		fprintf(stderr, "Init should reject negative bucket count\n");
		if (rc == 0)
			hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	return TEST_PASSED;
}

/* Test: Boundary bucket counts */
static int
test_boundary_bucket_counts(void)
{
	struct hash_engine engine;
	int rc;

	/* Minimum bucket count */
	rc = hash_engine_init(&engine, MIN_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "Init failed with MIN_BUCKET_COUNT\n");
		return TEST_FAILED;
	}
	hash_engine_destroy(&engine);

	/* Just below minimum */
	rc = hash_engine_init(&engine, MIN_BUCKET_COUNT - 1);
	/* Should round up to MIN_BUCKET_COUNT */
	if (rc != 0) {
		fprintf(stderr, "Init failed with below-min bucket count\n");
		return TEST_FAILED;
	}
	hash_engine_destroy(&engine);

	/* Maximum bucket count */
	rc = hash_engine_init(&engine, MAX_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "Init failed with MAX_BUCKET_COUNT\n");
		return TEST_FAILED;
	}
	hash_engine_destroy(&engine);

	/* Above maximum */
	rc = hash_engine_init(&engine, MAX_BUCKET_COUNT + 1);
	/* Implementation should handle or round down */
	if (rc == 0) {
		hash_engine_destroy(&engine);
	}
	/* Either success (rounded down) or failure is acceptable */

	return TEST_PASSED;
}

/* Test: Keys with identical content but different addresses */
static int
test_key_content_vs_pointer(void)
{
	struct hash_engine engine;
	char key1[] = "identical_content";
	char key2[] = "identical_content";
	const char *value1 = "value1";
	const char *value2 = "value2";
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert with key1 */
	rc = hash_put(&engine, key1, strlen(key1), value1, strlen(value1));
	if (rc != 0) {
		fprintf(stderr, "Failed to insert with key1\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Get with key2 (same content, different pointer) */
	rc = hash_get(&engine, key2, strlen(key2), &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != strlen(value1)
	    || memcmp(retrieved_value, value1, strlen(value1)) != 0) {
		fprintf(stderr,
			"Failed to retrieve with equivalent key pointer\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Update with key2 */
	rc = hash_put(&engine, key2, strlen(key2), value2, strlen(value2));
	if (rc != 0) {
		fprintf(stderr, "Failed to update with key2\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify update worked */
	rc = hash_get(&engine, key1, strlen(key1), &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != strlen(value2)
	    || memcmp(retrieved_value, value2, strlen(value2)) != 0) {
		fprintf(stderr, "Update with equivalent key failed\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Keys that differ only in the last byte */
static int
test_similar_keys(void)
{
	struct hash_engine engine;
	char key1[] = "similar_key_1";
	char key2[] = "similar_key_2";
	char key3[] = "similar_key_3";
	const char *value1 = "value1";
	const char *value2 = "value2";
	const char *value3 = "value3";
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert all three similar keys */
	rc = hash_put(&engine, key1, strlen(key1), value1, strlen(value1));
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	rc = hash_put(&engine, key2, strlen(key2), value2, strlen(value2));
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	rc = hash_put(&engine, key3, strlen(key3), value3, strlen(value3));
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify each retrieves the correct value */
	rc = hash_get(&engine, key1, strlen(key1), &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != strlen(value1)
	    || memcmp(retrieved_value, value1, strlen(value1)) != 0) {
		fprintf(stderr, "Failed to retrieve correct value for key1\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	rc = hash_get(&engine, key2, strlen(key2), &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != strlen(value2)
	    || memcmp(retrieved_value, value2, strlen(value2)) != 0) {
		fprintf(stderr, "Failed to retrieve correct value for key2\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	rc = hash_get(&engine, key3, strlen(key3), &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != strlen(value3)
	    || memcmp(retrieved_value, value3, strlen(value3)) != 0) {
		fprintf(stderr, "Failed to retrieve correct value for key3\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Keys with special characters and binary data */
static int
test_special_character_keys(void)
{
	struct hash_engine engine;
	char binary_key[16];
	const char *value = "value";
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;
	int i;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Create binary key with all byte values 0-15 */
	for (i = 0; i < 16; i++) {
		binary_key[i] = (char)i;
	}

	rc = hash_put(&engine, binary_key, sizeof(binary_key), value,
		      strlen(value));
	if (rc != 0) {
		fprintf(stderr, "Failed to insert binary key\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	rc = hash_get(&engine, binary_key, sizeof(binary_key), &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != strlen(value)
	    || memcmp(retrieved_value, value, strlen(value)) != 0) {
		fprintf(stderr, "Failed to retrieve with binary key\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Very long keys */
static int
test_very_long_keys(void)
{
	struct hash_engine engine;
	char *long_key;
	const char *value = "value";
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;
	size_t i;
	const size_t KEY_SIZE = 16384;

	long_key = malloc(KEY_SIZE);
	if (!long_key) {
		return TEST_FAILED;
	}

	/* Fill with pattern */
	for (i = 0; i < KEY_SIZE; i++) {
		long_key[i] = (char)('A' + (i % 26));
	}

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		free(long_key);
		return TEST_FAILED;
	}

	rc = hash_put(&engine, long_key, KEY_SIZE, value, strlen(value));
	if (rc != 0) {
		fprintf(stderr, "Failed to insert very long key\n");
		hash_engine_destroy(&engine);
		free(long_key);
		return TEST_FAILED;
	}

	rc = hash_get(&engine, long_key, KEY_SIZE, &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != strlen(value)
	    || memcmp(retrieved_value, value, strlen(value)) != 0) {
		fprintf(stderr, "Failed to retrieve with very long key\n");
		hash_engine_destroy(&engine);
		free(long_key);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	free(long_key);
	return TEST_PASSED;
}

/* Test: Very long values */
static int
test_very_long_values(void)
{
	struct hash_engine engine;
	const char *key = "key";
	char *long_value;
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;
	size_t i;
	const size_t VALUE_SIZE = 131072;

	long_value = malloc(VALUE_SIZE);
	if (!long_value) {
		return TEST_FAILED;
	}

	/* Fill with pattern */
	for (i = 0; i < VALUE_SIZE; i++) {
		long_value[i] = (char)('0' + (i % 10));
	}

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		free(long_value);
		return TEST_FAILED;
	}

	rc = hash_put(&engine, key, strlen(key), long_value, VALUE_SIZE);
	if (rc != 0) {
		fprintf(stderr, "Failed to insert very long value\n");
		hash_engine_destroy(&engine);
		free(long_value);
		return TEST_FAILED;
	}

	rc = hash_get(&engine, key, strlen(key), &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != VALUE_SIZE
	    || memcmp(retrieved_value, long_value, VALUE_SIZE) != 0) {
		fprintf(stderr, "Failed to retrieve very long value\n");
		hash_engine_destroy(&engine);
		free(long_value);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	free(long_value);
	return TEST_PASSED;
}

/* Test: Delete and re-insert same key multiple times */
static int
test_delete_reinsert_cycles(void)
{
	struct hash_engine engine;
	const char *key = "cycle_key";
	char value_buf[32];
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;
	int i;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	for (i = 0; i < 100; i++) {
		snprintf(value_buf, sizeof(value_buf), "cycle_value_%d", i);

		/* Insert */
		rc = hash_put(&engine, key, strlen(key), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr, "Insert failed at cycle %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		/* Verify */
		rc = hash_get(&engine, key, strlen(key), &retrieved_value,
			      &retrieved_len);
		if (rc != 0 || retrieved_len != strlen(value_buf)
		    || memcmp(retrieved_value, value_buf, strlen(value_buf))
			   != 0) {
			fprintf(stderr, "Get failed at cycle %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		/* Delete */
		rc = hash_delete(&engine, key, strlen(key));
		if (rc != 0) {
			fprintf(stderr, "Delete failed at cycle %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Operations at exact load factor boundaries */
static int
test_load_factor_boundaries(void)
{
	struct hash_engine engine;
	char key_buf[32];
	const char *value = "boundary_value";
	uint32_t initial_buckets;
	uint32_t current_buckets;
	int rc;
	int i;
	int max_items_before_resize;

	rc = hash_engine_init(&engine, INITIAL_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	rc = hash_engine_get_stats(&engine, NULL, &initial_buckets, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Insert up to but not exceeding 75% load factor */
	max_items_before_resize = (int)(initial_buckets * MAX_LOAD_FACTOR);

	for (i = 0; i < max_items_before_resize; i++) {
		snprintf(key_buf, sizeof(key_buf), "boundary_key_%d", i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value,
			      strlen(value));
		if (rc != 0) {
			fprintf(stderr, "Insert failed at boundary, item %d\n",
				i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	/* Check that we haven't resized yet */
	rc = hash_engine_get_stats(&engine, NULL, &current_buckets, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	if (current_buckets != initial_buckets) {
		fprintf(stderr,
			"Unexpected resize before exceeding load factor\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Insert one more to trigger resize */
	snprintf(key_buf, sizeof(key_buf), "boundary_key_%d", i);
	rc = hash_put(&engine, key_buf, strlen(key_buf), value, strlen(value));
	if (rc != 0) {
		fprintf(stderr, "Insert failed triggering resize\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify resize occurred */
	rc = hash_engine_get_stats(&engine, NULL, &current_buckets, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	if (current_buckets <= initial_buckets) {
		fprintf(stderr, "Resize did not occur at load factor\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

int
main(void)
{
	printf("===== Hash Table Edge Cases Tests =====\n\n");

	RUN_TEST(test_null_pointers);
	RUN_TEST(test_zero_length_key);
	RUN_TEST(test_zero_length_value);
	RUN_TEST(test_invalid_bucket_counts);
	RUN_TEST(test_boundary_bucket_counts);
	RUN_TEST(test_key_content_vs_pointer);
	RUN_TEST(test_similar_keys);
	RUN_TEST(test_special_character_keys);
	RUN_TEST(test_very_long_keys);
	RUN_TEST(test_very_long_values);
	RUN_TEST(test_delete_reinsert_cycles);
	RUN_TEST(test_load_factor_boundaries);

	printf("\n========================================\n");
	printf("Tests run: %d\n", tests_run);
	printf("Tests passed: %d\n", tests_passed);
	printf("Tests failed: %d\n", tests_failed);
	printf("========================================\n");

	return tests_failed > 0 ? 1 : 0;
}
