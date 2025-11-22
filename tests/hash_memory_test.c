/**
 * @file hash_memory_test.c
 * @brief Memory safety validation tests for hash table
 *
 * Tests for memory leaks, double-frees, use-after-free, and proper cleanup.
 * These tests are designed to be run under Valgrind and AddressSanitizer.
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

/* Test: Basic init and destroy cycle */
static int
test_clean_init_destroy(void)
{
	struct hash_engine engine;
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "Init failed\n");
		return TEST_FAILED;
	}

	rc = hash_engine_destroy(&engine);
	if (rc != 0) {
		fprintf(stderr, "Destroy failed\n");
		return TEST_FAILED;
	}

	return TEST_PASSED;
}

/* Test: Multiple init/destroy cycles */
static int
test_repeated_init_destroy(void)
{
	struct hash_engine engine;
	int rc;
	int i;

	for (i = 0; i < 100; i++) {
		rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
		if (rc != 0) {
			fprintf(stderr, "Init failed on cycle %d\n", i);
			return TEST_FAILED;
		}

		rc = hash_engine_destroy(&engine);
		if (rc != 0) {
			fprintf(stderr, "Destroy failed on cycle %d\n", i);
			return TEST_FAILED;
		}
	}

	return TEST_PASSED;
}

/* Test: Insert and destroy without delete (check for leaks) */
static int
test_destroy_with_data(void)
{
	struct hash_engine engine;
	char key_buf[32];
	char value_buf[64];
	int rc;
	int i;
	const int NUM_ITEMS = 50;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert items */
	for (i = 0; i < NUM_ITEMS; i++) {
		snprintf(key_buf, sizeof(key_buf), "leak_test_key_%d", i);
		snprintf(value_buf, sizeof(value_buf),
			 "leak_test_value_%d_with_some_extra_data", i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr, "Insert failed at item %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	/* Destroy without deleting - should free all memory */
	rc = hash_engine_destroy(&engine);
	if (rc != 0) {
		fprintf(stderr, "Destroy with data failed\n");
		return TEST_FAILED;
	}

	return TEST_PASSED;
}

/* Test: Multiple inserts, updates, deletes to check for cumulative leaks */
static int
test_insert_update_delete_cycles(void)
{
	struct hash_engine engine;
	const char *key = "memory_test_key";
	char value_buf[128];
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;
	int i;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Perform many insert/update/delete cycles */
	for (i = 0; i < 500; i++) {
		/* Insert with progressively larger values */
		snprintf(value_buf, sizeof(value_buf),
			 "iteration_%d_with_data_", i);
		memset(value_buf + strlen(value_buf), 'X',
		       sizeof(value_buf) - strlen(value_buf) - 1);
		value_buf[sizeof(value_buf) - 1] = '\0';

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
		if (rc != 0) {
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

/* Test: Verify no use-after-free on retrieved pointers */
static int
test_pointer_validity_after_update(void)
{
	struct hash_engine engine;
	const char *key = "uaf_test_key";
	const char *value1 = "first_value";
	const char *value2 = "second_value_different_size";
	const void *ptr1;
	const void *ptr2;
	size_t len1;
	size_t len2;
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert first value */
	rc = hash_put(&engine, key, strlen(key), value1, strlen(value1));
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Get pointer to first value */
	rc = hash_get(&engine, key, strlen(key), &ptr1, &len1);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Update with second value (different size) */
	rc = hash_put(&engine, key, strlen(key), value2, strlen(value2));
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Get pointer to second value */
	rc = hash_get(&engine, key, strlen(key), &ptr2, &len2);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify second value is correct */
	if (len2 != strlen(value2)
	    || memcmp(ptr2, value2, strlen(value2)) != 0) {
		fprintf(stderr, "Second value incorrect after update\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/*
	 * NOTE: ptr1 is now potentially dangling. We don't access it here,
	 * but ASan/Valgrind would catch if the implementation incorrectly
	 * left it pointing to freed memory and we tried to read it.
	 */

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Large number of items to test scaling behavior */
static int
test_large_scale_memory(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char value_buf[256];
	uint32_t item_count;
	int rc;
	int i;
	const int NUM_ITEMS = 1000;

	rc = hash_engine_init(&engine, INITIAL_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert many items to trigger multiple resizes */
	for (i = 0; i < NUM_ITEMS; i++) {
		snprintf(key_buf, sizeof(key_buf), "large_scale_key_%d", i);
		snprintf(value_buf, sizeof(value_buf),
			 "large_scale_value_%d_with_padding", i);
		memset(value_buf + strlen(value_buf), 'P',
		       sizeof(value_buf) - strlen(value_buf) - 1);
		value_buf[sizeof(value_buf) - 1] = '\0';

		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr,
				"Insert failed at item %d (possible memory "
				"issue)\n",
				i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	/* Verify count */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != NUM_ITEMS) {
		fprintf(stderr, "Item count mismatch: expected %d, got %u\n",
			NUM_ITEMS, item_count);
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Delete all items */
	for (i = 0; i < NUM_ITEMS; i++) {
		snprintf(key_buf, sizeof(key_buf), "large_scale_key_%d", i);
		rc = hash_delete(&engine, key_buf, strlen(key_buf));
		if (rc != 0) {
			fprintf(stderr, "Delete failed at item %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	/* Verify empty */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != 0) {
		fprintf(stderr, "Table not empty after deleting all items\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Resize-triggered memory operations */
static int
test_resize_memory_safety(void)
{
	struct hash_engine engine;
	char key_buf[32];
	char value_buf[64];
	uint32_t initial_buckets;
	uint32_t final_buckets;
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;
	int i;
	int num_items;

	/* Start with small bucket count */
	rc = hash_engine_init(&engine, INITIAL_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	rc = hash_engine_get_stats(&engine, NULL, &initial_buckets, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Insert enough to trigger multiple resizes */
	num_items = (int)(initial_buckets * MAX_LOAD_FACTOR * 4);
	for (i = 0; i < num_items; i++) {
		snprintf(key_buf, sizeof(key_buf), "resize_mem_key_%d", i);
		snprintf(value_buf, sizeof(value_buf), "resize_mem_val_%d",
			 i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr, "Insert failed at item %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	rc = hash_engine_get_stats(&engine, NULL, &final_buckets, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	if (final_buckets <= initial_buckets) {
		fprintf(stderr, "Resize did not occur\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Verify all data is still accessible after resizes */
	for (i = 0; i < num_items; i++) {
		snprintf(key_buf, sizeof(key_buf), "resize_mem_key_%d", i);
		snprintf(value_buf, sizeof(value_buf), "resize_mem_val_%d",
			 i);
		rc = hash_get(&engine, key_buf, strlen(key_buf),
			      &retrieved_value, &retrieved_len);
		if (rc != 0 || retrieved_len != strlen(value_buf)
		    || memcmp(retrieved_value, value_buf, retrieved_len)
			       != 0) {
			fprintf(stderr,
				"Data corrupted after resize for item %d\n",
				i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Tombstone handling doesn't leak memory */
static int
test_tombstone_memory(void)
{
	struct hash_engine engine;
	char key_buf[32];
	const char *value = "tombstone_test_value";
	uint32_t item_count;
	int rc;
	int i;
	const int NUM_CYCLES = 200;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/*
	 * Repeatedly insert and delete the same keys to create tombstones
	 * and then reuse those slots
	 */
	for (i = 0; i < NUM_CYCLES; i++) {
		/* Insert 20 keys */
		int j;
		for (j = 0; j < 20; j++) {
			snprintf(key_buf, sizeof(key_buf), "tombstone_%d", j);
			rc = hash_put(&engine, key_buf, strlen(key_buf), value,
				      strlen(value));
			if (rc != 0) {
				fprintf(stderr,
					"Insert failed at cycle %d, key %d\n",
					i, j);
				hash_engine_destroy(&engine);
				return TEST_FAILED;
			}
		}

		/* Verify count */
		rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
		if (rc != 0 || item_count != 20) {
			fprintf(stderr,
				"Item count wrong after insert in cycle %d\n",
				i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		/* Delete all 20 keys */
		for (j = 0; j < 20; j++) {
			snprintf(key_buf, sizeof(key_buf), "tombstone_%d", j);
			rc = hash_delete(&engine, key_buf, strlen(key_buf));
			if (rc != 0) {
				fprintf(stderr,
					"Delete failed at cycle %d, key %d\n",
					i, j);
				hash_engine_destroy(&engine);
				return TEST_FAILED;
			}
		}

		/* Verify empty */
		rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
		if (rc != 0 || item_count != 0) {
			fprintf(stderr,
				"Item count not zero after delete in cycle "
				"%d\n",
				i);
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
	printf("===== Hash Table Memory Safety Tests =====\n");
	printf("Run under Valgrind or ASan for full validation\n\n");

	RUN_TEST(test_clean_init_destroy);
	RUN_TEST(test_repeated_init_destroy);
	RUN_TEST(test_destroy_with_data);
	RUN_TEST(test_insert_update_delete_cycles);
	RUN_TEST(test_pointer_validity_after_update);
	RUN_TEST(test_large_scale_memory);
	RUN_TEST(test_resize_memory_safety);
	RUN_TEST(test_tombstone_memory);

	printf("\n========================================\n");
	printf("Tests run: %d\n", tests_run);
	printf("Tests passed: %d\n", tests_passed);
	printf("Tests failed: %d\n", tests_failed);
	printf("========================================\n");
	printf("\nNOTE: Run with:\n");
	printf("  valgrind --leak-check=full "
	       "./build/tests/tests/hash_memory_test.out\n");
	printf("  or compile with -fsanitize=address\n");

	return tests_failed > 0 ? 1 : 0;
}
