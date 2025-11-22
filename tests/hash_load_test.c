/**
 * @file hash_load_test.c
 * @brief Heavy load testing for hash table
 *
 * Tests hash table under heavy load conditions with large numbers of
 * operations, items, and sustained stress.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

/* Test: Insert and retrieve 100K items */
static int
test_load_100k_items(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char value_buf[128];
	const void *retrieved_value;
	size_t retrieved_len;
	uint32_t item_count;
	int rc;
	int i;
	const int NUM_ITEMS = 100000;

	printf("\n  Inserting %d items...", NUM_ITEMS);
	fflush(stdout);

	rc = hash_engine_init(&engine, INITIAL_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert items */
	for (i = 0; i < NUM_ITEMS; i++) {
		snprintf(key_buf, sizeof(key_buf), "load_key_%d", i);
		snprintf(value_buf, sizeof(value_buf),
			 "load_value_%d_with_data", i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr, "\n  Insert failed at item %d: %d\n", i,
				rc);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		if (i % 10000 == 0 && i > 0) {
			printf("\n  Progress: %d items...", i);
			fflush(stdout);
		}
	}

	printf("\n  Verifying count...");
	fflush(stdout);

	/* Verify count */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != NUM_ITEMS) {
		fprintf(stderr,
			"\n  Item count mismatch: expected %d, got %u\n",
			NUM_ITEMS, item_count);
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	printf("\n  Retrieving items...");
	fflush(stdout);

	/* Retrieve and verify all items */
	for (i = 0; i < NUM_ITEMS; i++) {
		snprintf(key_buf, sizeof(key_buf), "load_key_%d", i);
		snprintf(value_buf, sizeof(value_buf),
			 "load_value_%d_with_data", i);
		rc = hash_get(&engine, key_buf, strlen(key_buf),
			      &retrieved_value, &retrieved_len);
		if (rc != 0 || retrieved_len != strlen(value_buf)
		    || memcmp(retrieved_value, value_buf, retrieved_len) != 0) {
			fprintf(stderr, "\n  Retrieval failed at item %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		if (i % 10000 == 0 && i > 0) {
			printf("\n  Verified: %d items...", i);
			fflush(stdout);
		}
	}

	printf("\n  Deleting items...");
	fflush(stdout);

	/* Delete all items */
	for (i = 0; i < NUM_ITEMS; i++) {
		snprintf(key_buf, sizeof(key_buf), "load_key_%d", i);
		rc = hash_delete(&engine, key_buf, strlen(key_buf));
		if (rc != 0) {
			fprintf(stderr, "\n  Delete failed at item %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		if (i % 10000 == 0 && i > 0) {
			printf("\n  Deleted: %d items...", i);
			fflush(stdout);
		}
	}

	/* Verify empty */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != 0) {
		fprintf(stderr, "\n  Table not empty after delete all\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	printf("\n  Complete!");
	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Sustained mixed operations (1M operations) */
static int
test_load_1m_operations(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char value_buf[128];
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;
	int i;
	int key_id;
	int operation;
	int success_count;
	const int NUM_OPS = 1000000;
	const int KEY_SPACE = 50000;

	printf("\n  Performing %d mixed operations...", NUM_OPS);
	fflush(stdout);

	rc = hash_engine_init(&engine, INITIAL_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	srand((unsigned int)time(NULL));
	success_count = 0;

	for (i = 0; i < NUM_OPS; i++) {
		key_id = rand() % KEY_SPACE;
		operation = rand() % 3; /* 0=put, 1=get, 2=delete */

		snprintf(key_buf, sizeof(key_buf), "mixed_key_%d", key_id);
		snprintf(value_buf, sizeof(value_buf), "mixed_value_%d",
			 key_id);

		if (operation == 0) { /* PUT */
			rc = hash_put(&engine, key_buf, strlen(key_buf),
				      value_buf, strlen(value_buf));
			if (rc == 0)
				success_count++;
		} else if (operation == 1) { /* GET */
			rc = hash_get(&engine, key_buf, strlen(key_buf),
				      &retrieved_value, &retrieved_len);
			if (rc == 0)
				success_count++;
		} else { /* DELETE */
			rc = hash_delete(&engine, key_buf, strlen(key_buf));
			if (rc == 0)
				success_count++;
		}

		if (i % 100000 == 0 && i > 0) {
			printf("\n  Progress: %d operations, %d successful...",
			       i, success_count);
			fflush(stdout);
		}
	}

	printf("\n  Total successful operations: %d/%d", success_count,
	       NUM_OPS);
	printf("\n  Complete!");
	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Stress test with varying item sizes */
static int
test_load_varying_sizes(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char *value_buf;
	const void *retrieved_value;
	size_t retrieved_len;
	size_t value_size;
	int rc;
	int i;
	const int NUM_ITEMS = 10000;

	printf("\n  Testing with varying value sizes (1B to 64KB)...");
	fflush(stdout);

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert items with varying sizes */
	for (i = 0; i < NUM_ITEMS; i++) {
		snprintf(key_buf, sizeof(key_buf), "varsize_key_%d", i);

		/* Value size grows from 1 byte to 64KB */
		value_size = 1 + (i * 64 * 1024 / NUM_ITEMS);
		value_buf = malloc(value_size);
		if (!value_buf) {
			fprintf(stderr, "\n  Malloc failed at item %d\n", i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		memset(value_buf, 'V', value_size);

		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      value_size);
		if (rc != 0) {
			fprintf(stderr, "\n  Insert failed at item %d\n", i);
			free(value_buf);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		free(value_buf);

		if (i % 1000 == 0 && i > 0) {
			printf("\n  Progress: %d items...", i);
			fflush(stdout);
		}
	}

	printf("\n  Verifying items...");
	fflush(stdout);

	/* Verify all items */
	for (i = 0; i < NUM_ITEMS; i++) {
		snprintf(key_buf, sizeof(key_buf), "varsize_key_%d", i);
		value_size = 1 + (i * 64 * 1024 / NUM_ITEMS);

		rc = hash_get(&engine, key_buf, strlen(key_buf),
			      &retrieved_value, &retrieved_len);
		if (rc != 0 || retrieved_len != value_size) {
			fprintf(stderr,
				"\n  Retrieval failed at item %d: rc=%d "
				"expected_size=%zu got_size=%zu\n",
				i, rc, value_size, retrieved_len);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		if (i % 1000 == 0 && i > 0) {
			printf("\n  Verified: %d items...", i);
			fflush(stdout);
		}
	}

	printf("\n  Complete!");
	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Repeated resize cycles under load */
static int
test_load_resize_stress(void)
{
	struct hash_engine engine;
	char key_buf[64];
	const char *value = "resize_stress_value";
	uint32_t initial_buckets;
	uint32_t current_buckets;
	int rc;
	int i;
	int cycle;
	const int ITEMS_PER_CYCLE = 1000;
	const int NUM_CYCLES = 50;

	printf("\n  Testing resize under stress (%d cycles)...", NUM_CYCLES);
	fflush(stdout);

	rc = hash_engine_init(&engine, INITIAL_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	rc = hash_engine_get_stats(&engine, NULL, &initial_buckets, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	for (cycle = 0; cycle < NUM_CYCLES; cycle++) {
		/* Insert items */
		for (i = 0; i < ITEMS_PER_CYCLE; i++) {
			snprintf(key_buf, sizeof(key_buf),
				 "resize_stress_%d_%d", cycle, i);
			rc = hash_put(&engine, key_buf, strlen(key_buf), value,
				      strlen(value));
			if (rc != 0) {
				fprintf(stderr,
					"\n  Insert failed at cycle %d item "
					"%d\n",
					cycle, i);
				hash_engine_destroy(&engine);
				return TEST_FAILED;
			}
		}

		/* Delete half the items from this cycle */
		for (i = 0; i < ITEMS_PER_CYCLE / 2; i++) {
			snprintf(key_buf, sizeof(key_buf),
				 "resize_stress_%d_%d", cycle, i);
			rc = hash_delete(&engine, key_buf, strlen(key_buf));
			if (rc != 0) {
				fprintf(stderr,
					"\n  Delete failed at cycle %d item "
					"%d\n",
					cycle, i);
				hash_engine_destroy(&engine);
				return TEST_FAILED;
			}
		}

		if (cycle % 10 == 0 && cycle > 0) {
			printf("\n  Completed cycle %d...", cycle);
			fflush(stdout);
		}
	}

	rc = hash_engine_get_stats(&engine, NULL, &current_buckets, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	printf("\n  Buckets: initial=%u, final=%u", initial_buckets,
	       current_buckets);
	printf("\n  Complete!");
	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Pathological hash collision scenario */
static int
test_load_collision_stress(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char value_buf[64];
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;
	int i;
	const int NUM_ITEMS = 5000;

	printf("\n  Testing collision stress with similar keys...");
	fflush(stdout);

	rc = hash_engine_init(&engine, 64); /* Small table to force collisions
					     */
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert items with keys likely to collide */
	for (i = 0; i < NUM_ITEMS; i++) {
		snprintf(key_buf, sizeof(key_buf), "collision_key_%08d", i);
		snprintf(value_buf, sizeof(value_buf), "collision_val_%d", i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr,
				"\n  Insert failed at item %d (collision "
				"handling issue?)\n",
				i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		if (i % 500 == 0 && i > 0) {
			printf("\n  Progress: %d items...", i);
			fflush(stdout);
		}
	}

	printf("\n  Verifying all items...");
	fflush(stdout);

	/* Verify all items can be retrieved correctly */
	for (i = 0; i < NUM_ITEMS; i++) {
		snprintf(key_buf, sizeof(key_buf), "collision_key_%08d", i);
		snprintf(value_buf, sizeof(value_buf), "collision_val_%d", i);
		rc = hash_get(&engine, key_buf, strlen(key_buf),
			      &retrieved_value, &retrieved_len);
		if (rc != 0 || retrieved_len != strlen(value_buf)
		    || memcmp(retrieved_value, value_buf, retrieved_len) != 0) {
			fprintf(stderr,
				"\n  Retrieval failed at item %d (collision "
				"corruption?)\n",
				i);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		if (i % 500 == 0 && i > 0) {
			printf("\n  Verified: %d items...", i);
			fflush(stdout);
		}
	}

	printf("\n  Complete!");
	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

int
main(void)
{
	printf("===== Hash Table Load Tests =====\n");
	printf("WARNING: These tests may take several minutes\n\n");

	RUN_TEST(test_load_100k_items);
	RUN_TEST(test_load_1m_operations);
	RUN_TEST(test_load_varying_sizes);
	RUN_TEST(test_load_resize_stress);
	RUN_TEST(test_load_collision_stress);

	printf("\n========================================\n");
	printf("Tests run: %d\n", tests_run);
	printf("Tests passed: %d\n", tests_passed);
	printf("Tests failed: %d\n", tests_failed);
	printf("========================================\n");

	return tests_failed > 0 ? 1 : 0;
}
