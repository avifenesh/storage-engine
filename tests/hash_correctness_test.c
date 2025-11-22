/**
 * @file hash_correctness_test.c
 * @brief Comprehensive correctness tests for hash table implementation
 *
 * Tests edge cases, boundary conditions, and basic correctness properties.
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

/* Test: Initialize engine with various bucket counts */
static int
test_init_various_sizes(void)
{
	struct hash_engine engine;
	int rc;

	/* Test minimum bucket count */
	rc = hash_engine_init(&engine, MIN_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "Failed to init with MIN_BUCKET_COUNT\n");
		return TEST_FAILED;
	}
	hash_engine_destroy(&engine);

	/* Test default bucket count */
	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "Failed to init with DEFAULT_BUCKET_COUNT\n");
		return TEST_FAILED;
	}
	hash_engine_destroy(&engine);

	/* Test maximum bucket count */
	rc = hash_engine_init(&engine, MAX_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "Failed to init with MAX_BUCKET_COUNT\n");
		return TEST_FAILED;
	}
	hash_engine_destroy(&engine);

	/* Test power-of-two enforcement (should round up) */
	rc = hash_engine_init(&engine, 100);
	if (rc != 0) {
		fprintf(stderr, "Failed to init with non-power-of-two\n");
		return TEST_FAILED;
	}
	hash_engine_destroy(&engine);

	return TEST_PASSED;
}

/* Test: Operations on empty table */
static int
test_empty_table_operations(void)
{
	struct hash_engine engine;
	const void *value = NULL;
	size_t value_len = 0;
	const char *key = "nonexistent";
	uint32_t item_count;
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Get from empty table should return -ENOENT */
	rc = hash_get(&engine, key, strlen(key), &value, &value_len);
	if (rc != -ENOENT) {
		fprintf(stderr, "Get from empty table should return -ENOENT\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Delete from empty table should return -ENOENT */
	rc = hash_delete(&engine, key, strlen(key));
	if (rc != -ENOENT) {
		fprintf(stderr,
			"Delete from empty table should return -ENOENT\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify item count is zero */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != 0) {
		fprintf(stderr, "Empty table should have zero items\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Single element operations */
static int
test_single_element(void)
{
	struct hash_engine engine;
	const char *key = "test_key";
	const char *value = "test_value";
	const void *retrieved_value = NULL;
	size_t retrieved_len = 0;
	uint32_t item_count;
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert single element */
	rc = hash_put(&engine, key, strlen(key), value, strlen(value));
	if (rc != 0) {
		fprintf(stderr, "Failed to insert single element\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify item count is 1 */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != 1) {
		fprintf(stderr, "Item count should be 1 after insert\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Retrieve the element */
	rc = hash_get(&engine, key, strlen(key), &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != strlen(value) ||
	    memcmp(retrieved_value, value, retrieved_len) != 0) {
		fprintf(stderr, "Failed to retrieve single element\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Delete the element */
	rc = hash_delete(&engine, key, strlen(key));
	if (rc != 0) {
		fprintf(stderr, "Failed to delete single element\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify item count is 0 */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != 0) {
		fprintf(stderr, "Item count should be 0 after delete\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify element is gone */
	rc = hash_get(&engine, key, strlen(key), &retrieved_value,
		      &retrieved_len);
	if (rc != -ENOENT) {
		fprintf(stderr, "Get after delete should return -ENOENT\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Update existing key */
static int
test_update_existing_key(void)
{
	struct hash_engine engine;
	const char *key = "update_key";
	const char *value1 = "original_value";
	const char *value2 = "updated_value";
	const void *retrieved_value = NULL;
	size_t retrieved_len = 0;
	uint32_t item_count;
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert initial value */
	rc = hash_put(&engine, key, strlen(key), value1, strlen(value1));
	if (rc != 0) {
		fprintf(stderr, "Failed to insert initial value\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Update with new value */
	rc = hash_put(&engine, key, strlen(key), value2, strlen(value2));
	if (rc != 0) {
		fprintf(stderr, "Failed to update value\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify item count is still 1 (update, not insert) */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != 1) {
		fprintf(stderr, "Item count should be 1 after update\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify new value is retrieved */
	rc = hash_get(&engine, key, strlen(key), &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != strlen(value2) ||
	    memcmp(retrieved_value, value2, retrieved_len) != 0) {
		fprintf(stderr, "Failed to retrieve updated value\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Multiple unique keys */
static int
test_multiple_keys(void)
{
	struct hash_engine engine;
	char key_buf[32];
	char value_buf[32];
	const void *retrieved_value = NULL;
	size_t retrieved_len = 0;
	uint32_t item_count;
	int rc;
	int i;
	const int NUM_KEYS = 100;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert multiple keys */
	for (i = 0; i < NUM_KEYS; i++) {
		snprintf(key_buf, sizeof(key_buf), "key_%d", i);
		snprintf(value_buf, sizeof(value_buf), "value_%d", i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr, "Failed to insert key %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	/* Verify item count */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != NUM_KEYS) {
		fprintf(stderr, "Item count should be %d, got %u\n", NUM_KEYS,
			item_count);
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify all keys are retrievable with correct values */
	for (i = 0; i < NUM_KEYS; i++) {
		snprintf(key_buf, sizeof(key_buf), "key_%d", i);
		snprintf(value_buf, sizeof(value_buf), "value_%d", i);
		rc = hash_get(&engine, key_buf, strlen(key_buf),
			      &retrieved_value, &retrieved_len);
		if (rc != 0 || retrieved_len != strlen(value_buf) ||
		    memcmp(retrieved_value, value_buf, retrieved_len) != 0) {
			fprintf(stderr,
				"Failed to retrieve correct value for key %d\n",
				i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	/* Delete all keys */
	for (i = 0; i < NUM_KEYS; i++) {
		snprintf(key_buf, sizeof(key_buf), "key_%d", i);
		rc = hash_delete(&engine, key_buf, strlen(key_buf));
		if (rc != 0) {
			fprintf(stderr, "Failed to delete key %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	/* Verify item count is 0 */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != 0) {
		fprintf(stderr, "Item count should be 0 after deleting all\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Zero-length keys and values */
static int
test_zero_length(void)
{
	struct hash_engine engine;
	const char *empty_key = "";
	const char *empty_value = "";
	const char *key = "normal_key";
	const void *retrieved_value = NULL;
	size_t retrieved_len = 0;
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Try to insert empty key with normal value */
	rc = hash_put(&engine, empty_key, 0, "value", 5);
	if (rc == 0) {
		/* If implementation allows empty keys, verify retrieval */
		rc = hash_get(&engine, empty_key, 0, &retrieved_value,
			      &retrieved_len);
		if (rc != 0) {
			fprintf(stderr,
				"If empty keys allowed, should be retrievable\n");
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	/* Try to insert normal key with empty value */
	rc = hash_put(&engine, key, strlen(key), empty_value, 0);
	if (rc == 0) {
		/* If implementation allows empty values, verify retrieval */
		rc = hash_get(&engine, key, strlen(key), &retrieved_value,
			      &retrieved_len);
		if (rc != 0 || retrieved_len != 0) {
			fprintf(stderr,
				"If empty values allowed, should be retrievable\n");
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Large keys and values */
static int
test_large_data(void)
{
	struct hash_engine engine;
	char *large_key = NULL;
	char *large_value = NULL;
	const void *retrieved_value = NULL;
	size_t retrieved_len = 0;
	int rc;
	size_t i;
	const size_t KEY_SIZE = 4096;
	const size_t VALUE_SIZE = 65536;

	large_key = malloc(KEY_SIZE);
	large_value = malloc(VALUE_SIZE);
	if (!large_key || !large_value) {
		free(large_key);
		free(large_value);
		return TEST_FAILED;
	}

	/* Fill with pattern */
	for (i = 0; i < KEY_SIZE; i++) {
		large_key[i] = (char)('A' + (i % 26));
	}
	for (i = 0; i < VALUE_SIZE; i++) {
		large_value[i] = (char)('0' + (i % 10));
	}

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		free(large_key);
		free(large_value);
		return TEST_FAILED;
	}

	/* Insert large data */
	rc = hash_put(&engine, large_key, KEY_SIZE, large_value, VALUE_SIZE);
	if (rc != 0) {
		fprintf(stderr, "Failed to insert large data\n");
		hash_engine_destroy(&engine);
		free(large_key);
		free(large_value);
		return TEST_FAILED;
	}

	/* Retrieve and verify */
	rc = hash_get(&engine, large_key, KEY_SIZE, &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != VALUE_SIZE ||
	    memcmp(retrieved_value, large_value, VALUE_SIZE) != 0) {
		fprintf(stderr, "Failed to retrieve large data correctly\n");
		hash_engine_destroy(&engine);
		free(large_key);
		free(large_value);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	free(large_key);
	free(large_value);
	return TEST_PASSED;
}

/* Test: Duplicate key insertions */
static int
test_duplicate_keys(void)
{
	struct hash_engine engine;
	const char *key = "duplicate";
	const char *value1 = "first";
	const char *value2 = "second";
	const char *value3 = "third";
	const void *retrieved_value = NULL;
	size_t retrieved_len = 0;
	uint32_t item_count;
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert same key multiple times */
	rc = hash_put(&engine, key, strlen(key), value1, strlen(value1));
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	rc = hash_put(&engine, key, strlen(key), value2, strlen(value2));
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	rc = hash_put(&engine, key, strlen(key), value3, strlen(value3));
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify only one item exists */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != 1) {
		fprintf(stderr,
			"Duplicate inserts should maintain count of 1\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify latest value is stored */
	rc = hash_get(&engine, key, strlen(key), &retrieved_value,
		      &retrieved_len);
	if (rc != 0 || retrieved_len != strlen(value3) ||
	    memcmp(retrieved_value, value3, retrieved_len) != 0) {
		fprintf(stderr, "Should retrieve latest value\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Delete non-existent key */
static int
test_delete_nonexistent(void)
{
	struct hash_engine engine;
	const char *key1 = "exists";
	const char *key2 = "does_not_exist";
	const char *value = "value";
	const void *retrieved_value = NULL;
	size_t retrieved_len = 0;
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert one key */
	rc = hash_put(&engine, key1, strlen(key1), value, strlen(value));
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Try to delete non-existent key */
	rc = hash_delete(&engine, key2, strlen(key2));
	if (rc != -ENOENT) {
		fprintf(stderr,
			"Deleting non-existent key should return -ENOENT\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify existing key is still there */
	rc = hash_get(&engine, key1, strlen(key1), &retrieved_value,
		      &retrieved_len);
	if (rc != 0) {
		fprintf(stderr,
			"Existing key should still be retrievable\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Resize trigger (load factor) */
static int
test_resize_trigger(void)
{
	struct hash_engine engine;
	char key_buf[32];
	char value_buf[32];
	uint32_t initial_bucket_count, current_bucket_count;
	int rc;
	int i;
	int num_elements;

	/* Start with small bucket count to trigger resize */
	rc = hash_engine_init(&engine, INITIAL_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	rc = hash_engine_get_stats(&engine, NULL, &initial_bucket_count, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Insert enough elements to trigger resize (>75% load factor) */
	num_elements = (int)(initial_bucket_count * MAX_LOAD_FACTOR) + 10;
	for (i = 0; i < num_elements; i++) {
		snprintf(key_buf, sizeof(key_buf), "resize_key_%d", i);
		snprintf(value_buf, sizeof(value_buf), "resize_value_%d", i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr, "Failed to insert key %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	/* Verify resize occurred */
	rc = hash_engine_get_stats(&engine, NULL, &current_bucket_count, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	if (current_bucket_count <= initial_bucket_count) {
		fprintf(stderr, "Resize should have occurred\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify all data is still accessible */
	for (i = 0; i < num_elements; i++) {
		const void *retrieved_value = NULL;
		size_t retrieved_len = 0;
		snprintf(key_buf, sizeof(key_buf), "resize_key_%d", i);
		snprintf(value_buf, sizeof(value_buf), "resize_value_%d", i);
		rc = hash_get(&engine, key_buf, strlen(key_buf),
			      &retrieved_value, &retrieved_len);
		if (rc != 0 || retrieved_len != strlen(value_buf) ||
		    memcmp(retrieved_value, value_buf, retrieved_len) != 0) {
			fprintf(stderr,
				"Data lost or corrupted after resize for key %d\n",
				i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Sequential insert, get, delete cycle */
static int
test_sequential_operations(void)
{
	struct hash_engine engine;
	const char *key = "sequential";
	const char *value = "value";
	const void *retrieved_value = NULL;
	size_t retrieved_len = 0;
	uint32_t item_count;
	int rc;
	int i;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Perform multiple cycles */
	for (i = 0; i < 10; i++) {
		/* Insert */
		rc = hash_put(&engine, key, strlen(key), value, strlen(value));
		if (rc != 0) {
			fprintf(stderr, "Insert failed in cycle %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		/* Verify count */
		rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
		if (rc != 0 || item_count != 1) {
			fprintf(stderr, "Item count wrong in cycle %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		/* Get */
		rc = hash_get(&engine, key, strlen(key), &retrieved_value,
			      &retrieved_len);
		if (rc != 0 || retrieved_len != strlen(value) ||
		    memcmp(retrieved_value, value, retrieved_len) != 0) {
			fprintf(stderr, "Get failed in cycle %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		/* Delete */
		rc = hash_delete(&engine, key, strlen(key));
		if (rc != 0) {
			fprintf(stderr, "Delete failed in cycle %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		/* Verify gone */
		rc = hash_get(&engine, key, strlen(key), &retrieved_value,
			      &retrieved_len);
		if (rc != -ENOENT) {
			fprintf(stderr,
				"Key should be gone after delete in cycle %d\n",
				i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		/* Verify count is 0 */
		rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
		if (rc != 0 || item_count != 0) {
			fprintf(stderr,
				"Item count should be 0 in cycle %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

int
main(void)
{
	printf("===== Hash Table Correctness Tests =====\n\n");

	RUN_TEST(test_init_various_sizes);
	RUN_TEST(test_empty_table_operations);
	RUN_TEST(test_single_element);
	RUN_TEST(test_update_existing_key);
	RUN_TEST(test_multiple_keys);
	RUN_TEST(test_zero_length);
	RUN_TEST(test_large_data);
	RUN_TEST(test_duplicate_keys);
	RUN_TEST(test_delete_nonexistent);
	RUN_TEST(test_resize_trigger);
	RUN_TEST(test_sequential_operations);

	printf("\n========================================\n");
	printf("Tests run: %d\n", tests_run);
	printf("Tests passed: %d\n", tests_passed);
	printf("Tests failed: %d\n", tests_failed);
	printf("========================================\n");

	return tests_failed > 0 ? 1 : 0;
}
