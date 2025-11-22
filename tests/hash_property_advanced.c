/**
 * @file hash_property_advanced.c
 * @brief Advanced property-based testing for hash table
 *
 * Tests hash table properties using oracle patterns, metamorphic testing,
 * and invariant checking.
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "storage/hash_engine.h"

#define TEST_PASSED 0
#define TEST_FAILED 1

#define ORACLE_SIZE 512
#define MAX_KEY_ID 2048
#define OPERATIONS_PER_TRIAL 512
#define NUM_TRIALS 10

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

/* Oracle slot for reference implementation */
struct oracle_slot {
	int key;
	char value[64];
	size_t value_len;
	bool present;
};

/* Test: Basic property-based oracle test */
static int
test_property_oracle(void)
{
	struct hash_engine engine;
	struct oracle_slot oracle[ORACLE_SIZE];
	char key_buf[32];
	char value_buf[64];
	const void *retrieved_value;
	size_t retrieved_len;
	uint32_t hash_count;
	int oracle_count;
	int rc;
	int trial;
	int op;
	int i;

	/* Initialize oracle */
	for (i = 0; i < ORACLE_SIZE; i++) {
		oracle[i].key = -1;
		oracle[i].value_len = 0;
		oracle[i].present = false;
	}

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	srand((unsigned int)time(NULL));

	for (trial = 0; trial < NUM_TRIALS; trial++) {
		for (op = 0; op < OPERATIONS_PER_TRIAL; op++) {
			int key_id = rand() % MAX_KEY_ID;
			int oracle_idx = key_id % ORACLE_SIZE;
			int operation = rand() % 3; /* 0=put, 1=get, 2=delete */

			snprintf(key_buf, sizeof(key_buf), "prop_key_%d",
				 key_id);

			if (operation == 0) { /* PUT */
				snprintf(value_buf, sizeof(value_buf),
					 "prop_val_%d_%d", key_id, op);
				rc = hash_put(&engine, key_buf, strlen(key_buf),
					      value_buf, strlen(value_buf));

				if (rc == 0) {
					/* Update oracle */
					oracle[oracle_idx].key = key_id;
					strcpy(oracle[oracle_idx].value,
					       value_buf);
					oracle[oracle_idx].value_len
					    = strlen(value_buf);
					oracle[oracle_idx].present = true;
				}

			} else if (operation == 1) { /* GET */
				rc = hash_get(&engine, key_buf, strlen(key_buf),
					      &retrieved_value,
					      &retrieved_len);

				/* Check against oracle */
				if (oracle[oracle_idx].present
				    && oracle[oracle_idx].key == key_id) {
					if (rc != 0
					    || retrieved_len
						       != oracle[oracle_idx]
								  .value_len
					    || memcmp(retrieved_value,
						      oracle[oracle_idx].value,
						      retrieved_len)
						       != 0) {
						fprintf(stderr,
							"Oracle mismatch on "
							"GET trial %d op %d\n",
							trial, op);
						hash_engine_destroy(&engine);
						return TEST_FAILED;
					}
				} else {
					if (rc != -ENOENT) {
						fprintf(stderr,
							"GET should return "
							"ENOENT trial %d op "
							"%d\n",
							trial, op);
						hash_engine_destroy(&engine);
						return TEST_FAILED;
					}
				}

			} else { /* DELETE */
				rc = hash_delete(&engine, key_buf,
						 strlen(key_buf));

				if (oracle[oracle_idx].present
				    && oracle[oracle_idx].key == key_id) {
					if (rc != 0) {
						fprintf(stderr,
							"DELETE failed when "
							"should succeed trial "
							"%d op %d\n",
							trial, op);
						hash_engine_destroy(&engine);
						return TEST_FAILED;
					}
					oracle[oracle_idx].present = false;
				} else {
					if (rc != -ENOENT) {
						fprintf(stderr,
							"DELETE should return "
							"ENOENT trial %d op "
							"%d\n",
							trial, op);
						hash_engine_destroy(&engine);
						return TEST_FAILED;
					}
				}
			}
		}
	}

	/* Final consistency check */
	rc = hash_engine_get_stats(&engine, &hash_count, NULL, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	oracle_count = 0;
	for (i = 0; i < ORACLE_SIZE; i++) {
		if (oracle[i].present)
			oracle_count++;
	}

	if ((int)hash_count != oracle_count) {
		fprintf(stderr,
			"Final count mismatch: hash=%u oracle=%d\n",
			hash_count, oracle_count);
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Metamorphic property - insertion order shouldn't matter */
static int
test_metamorphic_insertion_order(void)
{
	struct hash_engine engine1;
	struct hash_engine engine2;
	const char *keys[] = { "key1", "key2", "key3", "key4", "key5" };
	const char *values[]
	    = { "value1", "value2", "value3", "value4", "value5" };
	const void *retrieved1;
	const void *retrieved2;
	size_t len1;
	size_t len2;
	int rc;
	int i;
	const int NUM_KEYS = 5;

	rc = hash_engine_init(&engine1, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	rc = hash_engine_init(&engine2, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		hash_engine_destroy(&engine1);
		return TEST_FAILED;
	}

	/* Insert in forward order */
	for (i = 0; i < NUM_KEYS; i++) {
		rc = hash_put(&engine1, keys[i], strlen(keys[i]), values[i],
			      strlen(values[i]));
		if (rc != 0) {
			fprintf(stderr, "Insert failed in engine1\n");
			hash_engine_destroy(&engine1);
			hash_engine_destroy(&engine2);
			return TEST_FAILED;
		}
	}

	/* Insert in reverse order */
	for (i = NUM_KEYS - 1; i >= 0; i--) {
		rc = hash_put(&engine2, keys[i], strlen(keys[i]), values[i],
			      strlen(values[i]));
		if (rc != 0) {
			fprintf(stderr, "Insert failed in engine2\n");
			hash_engine_destroy(&engine1);
			hash_engine_destroy(&engine2);
			return TEST_FAILED;
		}
	}

	/* Verify both engines return same values for all keys */
	for (i = 0; i < NUM_KEYS; i++) {
		rc = hash_get(&engine1, keys[i], strlen(keys[i]), &retrieved1,
			      &len1);
		if (rc != 0) {
			fprintf(stderr, "Get failed in engine1 for key %d\n",
				i);
			hash_engine_destroy(&engine1);
			hash_engine_destroy(&engine2);
			return TEST_FAILED;
		}

		rc = hash_get(&engine2, keys[i], strlen(keys[i]), &retrieved2,
			      &len2);
		if (rc != 0) {
			fprintf(stderr, "Get failed in engine2 for key %d\n",
				i);
			hash_engine_destroy(&engine1);
			hash_engine_destroy(&engine2);
			return TEST_FAILED;
		}

		if (len1 != len2 || memcmp(retrieved1, retrieved2, len1) != 0) {
			fprintf(stderr,
				"Values differ between engines for key %d\n",
				i);
			hash_engine_destroy(&engine1);
			hash_engine_destroy(&engine2);
			return TEST_FAILED;
		}
	}

	hash_engine_destroy(&engine1);
	hash_engine_destroy(&engine2);
	return TEST_PASSED;
}

/* Test: Invariant - count always matches actual items */
static int
test_invariant_count_consistency(void)
{
	struct hash_engine engine;
	char key_buf[32];
	const char *value = "inv_value";
	uint32_t reported_count;
	int actual_count;
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;
	int i;
	int key_id;
	int operation;
	bool exists;
	const int NUM_OPS = 100;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	actual_count = 0;

	/* Perform random operations and track count */
	srand(42); /* Fixed seed for reproducibility */
	for (i = 0; i < NUM_OPS; i++) {
		key_id = rand() % 20;
		operation = rand() % 2; /* 0=put, 1=delete */

		snprintf(key_buf, sizeof(key_buf), "inv_key_%d", key_id);

		/* Check if key exists */
		rc = hash_get(&engine, key_buf, strlen(key_buf),
			      &retrieved_value, &retrieved_len);
		exists = (rc == 0);

		if (operation == 0) { /* PUT */
			rc = hash_put(&engine, key_buf, strlen(key_buf), value,
				      strlen(value));
			if (rc == 0 && !exists) {
				actual_count++;
			}
		} else { /* DELETE */
			rc = hash_delete(&engine, key_buf, strlen(key_buf));
			if (rc == 0 && exists) {
				actual_count--;
			}
		}

		/* Verify count invariant */
		rc = hash_engine_get_stats(&engine, &reported_count, NULL,
					   NULL);
		if (rc != 0) {
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}

		if ((int)reported_count != actual_count) {
			fprintf(stderr,
				"Count invariant violated at op %d: "
				"reported=%u actual=%d\n",
				i, reported_count, actual_count);
			hash_engine_destroy(&engine);
			return TEST_FAILED;
		}
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Idempotence - double delete returns ENOENT */
static int
test_idempotence_double_delete(void)
{
	struct hash_engine engine;
	const char *key = "idempotent_key";
	const char *value = "value";
	int rc;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	/* Insert */
	rc = hash_put(&engine, key, strlen(key), value, strlen(value));
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* First delete should succeed */
	rc = hash_delete(&engine, key, strlen(key));
	if (rc != 0) {
		fprintf(stderr, "First delete failed\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	/* Second delete should return -ENOENT */
	rc = hash_delete(&engine, key, strlen(key));
	if (rc != -ENOENT) {
		fprintf(stderr, "Double delete should return -ENOENT\n");
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

/* Test: Commutativity - put(k1,v1) + put(k2,v2) = put(k2,v2) + put(k1,v1) */
static int
test_commutativity_independent_keys(void)
{
	struct hash_engine engine1;
	struct hash_engine engine2;
	const char *key1 = "comm_key1";
	const char *key2 = "comm_key2";
	const char *value1 = "comm_value1";
	const char *value2 = "comm_value2";
	const void *ret1_k1;
	const void *ret1_k2;
	const void *ret2_k1;
	const void *ret2_k2;
	size_t len1_k1;
	size_t len1_k2;
	size_t len2_k1;
	size_t len2_k2;
	int rc;

	rc = hash_engine_init(&engine1, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	rc = hash_engine_init(&engine2, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		hash_engine_destroy(&engine1);
		return TEST_FAILED;
	}

	/* Engine1: put k1, then k2 */
	rc = hash_put(&engine1, key1, strlen(key1), value1, strlen(value1));
	if (rc != 0) {
		hash_engine_destroy(&engine1);
		hash_engine_destroy(&engine2);
		return TEST_FAILED;
	}
	rc = hash_put(&engine1, key2, strlen(key2), value2, strlen(value2));
	if (rc != 0) {
		hash_engine_destroy(&engine1);
		hash_engine_destroy(&engine2);
		return TEST_FAILED;
	}

	/* Engine2: put k2, then k1 */
	rc = hash_put(&engine2, key2, strlen(key2), value2, strlen(value2));
	if (rc != 0) {
		hash_engine_destroy(&engine1);
		hash_engine_destroy(&engine2);
		return TEST_FAILED;
	}
	rc = hash_put(&engine2, key1, strlen(key1), value1, strlen(value1));
	if (rc != 0) {
		hash_engine_destroy(&engine1);
		hash_engine_destroy(&engine2);
		return TEST_FAILED;
	}

	/* Retrieve from both and compare */
	rc = hash_get(&engine1, key1, strlen(key1), &ret1_k1, &len1_k1);
	if (rc != 0) {
		hash_engine_destroy(&engine1);
		hash_engine_destroy(&engine2);
		return TEST_FAILED;
	}
	rc = hash_get(&engine1, key2, strlen(key2), &ret1_k2, &len1_k2);
	if (rc != 0) {
		hash_engine_destroy(&engine1);
		hash_engine_destroy(&engine2);
		return TEST_FAILED;
	}
	rc = hash_get(&engine2, key1, strlen(key1), &ret2_k1, &len2_k1);
	if (rc != 0) {
		hash_engine_destroy(&engine1);
		hash_engine_destroy(&engine2);
		return TEST_FAILED;
	}
	rc = hash_get(&engine2, key2, strlen(key2), &ret2_k2, &len2_k2);
	if (rc != 0) {
		hash_engine_destroy(&engine1);
		hash_engine_destroy(&engine2);
		return TEST_FAILED;
	}

	/* Verify commutativity */
	if (len1_k1 != len2_k1 || memcmp(ret1_k1, ret2_k1, len1_k1) != 0) {
		fprintf(stderr, "Key1 values differ between engines\n");
		hash_engine_destroy(&engine1);
		hash_engine_destroy(&engine2);
		return TEST_FAILED;
	}
	if (len1_k2 != len2_k2 || memcmp(ret1_k2, ret2_k2, len1_k2) != 0) {
		fprintf(stderr, "Key2 values differ between engines\n");
		hash_engine_destroy(&engine1);
		hash_engine_destroy(&engine2);
		return TEST_FAILED;
	}

	hash_engine_destroy(&engine1);
	hash_engine_destroy(&engine2);
	return TEST_PASSED;
}

int
main(void)
{
	printf("===== Hash Table Advanced Property Tests =====\n\n");

	RUN_TEST(test_property_oracle);
	RUN_TEST(test_metamorphic_insertion_order);
	RUN_TEST(test_invariant_count_consistency);
	RUN_TEST(test_idempotence_double_delete);
	RUN_TEST(test_commutativity_independent_keys);

	printf("\n========================================\n");
	printf("Tests run: %d\n", tests_run);
	printf("Tests passed: %d\n", tests_passed);
	printf("Tests failed: %d\n", tests_failed);
	printf("========================================\n");

	return tests_failed > 0 ? 1 : 0;
}
