/**
 * @file hash_concurrent_test.c
 * @brief Concurrency and thread safety tests for hash table
 *
 * Tests hash table behavior under concurrent access from multiple threads.
 * Checks for data races, deadlocks, and consistency under concurrency.
 */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "storage/hash_engine.h"

#define TEST_PASSED 0
#define TEST_FAILED 1

#define NUM_THREADS 8
#define OPS_PER_THREAD 1000

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

/* Thread arguments for concurrent operations */
struct thread_args {
	struct hash_engine *engine;
	int thread_id;
	int *error_count;
	pthread_mutex_t *error_mutex;
};

/* Test: Concurrent reads (should be safe) */
static void *
concurrent_read_worker(void *arg)
{
	struct thread_args *args;
	char key_buf[64];
	const void *value;
	size_t value_len;
	int rc;
	int i;
	int local_errors;

	args = (struct thread_args *)arg;
	local_errors = 0;

	for (i = 0; i < OPS_PER_THREAD; i++) {
		snprintf(key_buf, sizeof(key_buf), "read_key_%d",
			 i % 100); /* Read from set of 100 keys */
		rc = hash_get(args->engine, key_buf, strlen(key_buf), &value,
			      &value_len);
		/* We expect some ENOENT, but no crashes */
		if (rc != 0 && rc != -ENOENT) {
			local_errors++;
		}
	}

	pthread_mutex_lock(args->error_mutex);
	*args->error_count += local_errors;
	pthread_mutex_unlock(args->error_mutex);

	return NULL;
}

static int
test_concurrent_reads(void)
{
	struct hash_engine engine;
	pthread_t threads[NUM_THREADS];
	struct thread_args args[NUM_THREADS];
	pthread_mutex_t error_mutex;
	int error_count;
	char key_buf[64];
	const char *value = "concurrent_read_value";
	int rc;
	int i;

	error_count = 0;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	pthread_mutex_init(&error_mutex, NULL);

	/* Pre-populate with some keys */
	for (i = 0; i < 100; i++) {
		snprintf(key_buf, sizeof(key_buf), "read_key_%d", i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value,
			      strlen(value));
		if (rc != 0) {
			fprintf(stderr, "Setup failed\n");
			hash_engine_destroy(&engine);
			pthread_mutex_destroy(&error_mutex);
			return TEST_FAILED;
		}
	}

	/* Launch reader threads */
	for (i = 0; i < NUM_THREADS; i++) {
		args[i].engine = &engine;
		args[i].thread_id = i;
		args[i].error_count = &error_count;
		args[i].error_mutex = &error_mutex;
		pthread_create(&threads[i], NULL, concurrent_read_worker,
			       &args[i]);
	}

	/* Wait for completion */
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	pthread_mutex_destroy(&error_mutex);
	hash_engine_destroy(&engine);

	if (error_count > 0) {
		fprintf(stderr, "Concurrent reads had %d errors\n",
			error_count);
		return TEST_FAILED;
	}

	return TEST_PASSED;
}

/* Test: Concurrent writes to different keys (should be safe) */
static void *
concurrent_write_unique_worker(void *arg)
{
	struct thread_args *args;
	char key_buf[64];
	char value_buf[64];
	int rc;
	int i;
	int local_errors;

	args = (struct thread_args *)arg;
	local_errors = 0;

	for (i = 0; i < OPS_PER_THREAD; i++) {
		/* Each thread writes to unique keys */
		snprintf(key_buf, sizeof(key_buf), "write_key_%d_%d",
			 args->thread_id, i);
		snprintf(value_buf, sizeof(value_buf), "write_val_%d_%d",
			 args->thread_id, i);
		rc = hash_put(args->engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			local_errors++;
		}
	}

	pthread_mutex_lock(args->error_mutex);
	*args->error_count += local_errors;
	pthread_mutex_unlock(args->error_mutex);

	return NULL;
}

static int
test_concurrent_writes_unique_keys(void)
{
	struct hash_engine engine;
	pthread_t threads[NUM_THREADS];
	struct thread_args args[NUM_THREADS];
	pthread_mutex_t error_mutex;
	int error_count;
	uint32_t item_count;
	int expected_count;
	int rc;
	int i;

	error_count = 0;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	pthread_mutex_init(&error_mutex, NULL);

	/* Launch writer threads */
	for (i = 0; i < NUM_THREADS; i++) {
		args[i].engine = &engine;
		args[i].thread_id = i;
		args[i].error_count = &error_count;
		args[i].error_mutex = &error_mutex;
		pthread_create(&threads[i], NULL,
			       concurrent_write_unique_worker, &args[i]);
	}

	/* Wait for completion */
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	/* Verify count */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	expected_count = NUM_THREADS * OPS_PER_THREAD;

	pthread_mutex_destroy(&error_mutex);
	hash_engine_destroy(&engine);

	if (error_count > 0) {
		fprintf(stderr, "Concurrent writes had %d errors\n",
			error_count);
		return TEST_FAILED;
	}

	if ((int)item_count != expected_count) {
		fprintf(stderr, "Item count mismatch: expected %d, got %u\n",
			expected_count, item_count);
		return TEST_FAILED;
	}

	return TEST_PASSED;
}

/* Test: Concurrent writes to same key (stress test) */
static void *
concurrent_write_contended_worker(void *arg)
{
	struct thread_args *args;
	const char *key = "contended_key";
	char value_buf[64];
	int rc;
	int i;
	int local_errors;

	args = (struct thread_args *)arg;
	local_errors = 0;

	for (i = 0; i < OPS_PER_THREAD; i++) {
		snprintf(value_buf, sizeof(value_buf), "thread_%d_val_%d",
			 args->thread_id, i);
		rc = hash_put(args->engine, key, strlen(key), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			local_errors++;
		}
	}

	pthread_mutex_lock(args->error_mutex);
	*args->error_count += local_errors;
	pthread_mutex_unlock(args->error_mutex);

	return NULL;
}

static int
test_concurrent_writes_same_key(void)
{
	struct hash_engine engine;
	pthread_t threads[NUM_THREADS];
	struct thread_args args[NUM_THREADS];
	pthread_mutex_t error_mutex;
	int error_count;
	const char *key = "contended_key";
	const void *final_value;
	size_t final_len;
	uint32_t item_count;
	int rc;
	int i;

	error_count = 0;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	pthread_mutex_init(&error_mutex, NULL);

	/* Launch writer threads all writing to same key */
	for (i = 0; i < NUM_THREADS; i++) {
		args[i].engine = &engine;
		args[i].thread_id = i;
		args[i].error_count = &error_count;
		args[i].error_mutex = &error_mutex;
		pthread_create(&threads[i], NULL,
			       concurrent_write_contended_worker, &args[i]);
	}

	/* Wait for completion */
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	/* Verify single item exists and is readable */
	rc = hash_engine_get_stats(&engine, &item_count, NULL, NULL);
	if (rc != 0 || item_count != 1) {
		fprintf(stderr, "Expected 1 item, got %u\n", item_count);
		pthread_mutex_destroy(&error_mutex);
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	rc = hash_get(&engine, key, strlen(key), &final_value, &final_len);
	if (rc != 0) {
		fprintf(stderr, "Failed to retrieve contended key\n");
		pthread_mutex_destroy(&error_mutex);
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	pthread_mutex_destroy(&error_mutex);
	hash_engine_destroy(&engine);

	if (error_count > 0) {
		fprintf(stderr, "Concurrent contended writes had %d errors\n",
			error_count);
		return TEST_FAILED;
	}

	return TEST_PASSED;
}

/* Test: Mixed concurrent operations */
static void *
concurrent_mixed_worker(void *arg)
{
	struct thread_args *args;
	char key_buf[64];
	char value_buf[64];
	const void *retrieved_value;
	size_t retrieved_len;
	int rc;
	int i;
	int operation;
	int local_errors;

	args = (struct thread_args *)arg;
	local_errors = 0;

	/* Seed per-thread random */
	srand((unsigned int)time(NULL) + args->thread_id);

	for (i = 0; i < OPS_PER_THREAD; i++) {
		int key_id = rand() % 200;
		operation = rand() % 3;

		snprintf(key_buf, sizeof(key_buf), "mixed_key_%d", key_id);
		snprintf(value_buf, sizeof(value_buf), "mixed_val_%d_%d",
			 args->thread_id, i);

		if (operation == 0) { /* PUT */
			rc = hash_put(args->engine, key_buf, strlen(key_buf),
				      value_buf, strlen(value_buf));
			if (rc != 0 && rc != -ENOMEM && rc != -ENOSPC) {
				local_errors++;
			}
		} else if (operation == 1) { /* GET */
			rc = hash_get(args->engine, key_buf, strlen(key_buf),
				      &retrieved_value, &retrieved_len);
			if (rc != 0 && rc != -ENOENT) {
				local_errors++;
			}
		} else { /* DELETE */
			rc = hash_delete(args->engine, key_buf,
					 strlen(key_buf));
			if (rc != 0 && rc != -ENOENT) {
				local_errors++;
			}
		}
	}

	pthread_mutex_lock(args->error_mutex);
	*args->error_count += local_errors;
	pthread_mutex_unlock(args->error_mutex);

	return NULL;
}

static int
test_concurrent_mixed_operations(void)
{
	struct hash_engine engine;
	pthread_t threads[NUM_THREADS];
	struct thread_args args[NUM_THREADS];
	pthread_mutex_t error_mutex;
	int error_count;
	int rc;
	int i;

	error_count = 0;

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	pthread_mutex_init(&error_mutex, NULL);

	/* Launch threads with mixed operations */
	for (i = 0; i < NUM_THREADS; i++) {
		args[i].engine = &engine;
		args[i].thread_id = i;
		args[i].error_count = &error_count;
		args[i].error_mutex = &error_mutex;
		pthread_create(&threads[i], NULL, concurrent_mixed_worker,
			       &args[i]);
	}

	/* Wait for completion */
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	pthread_mutex_destroy(&error_mutex);
	hash_engine_destroy(&engine);

	if (error_count > 0) {
		fprintf(stderr, "Concurrent mixed ops had %d errors\n",
			error_count);
		return TEST_FAILED;
	}

	return TEST_PASSED;
}

/* Test: Concurrent resize stress */
static void *
concurrent_resize_worker(void *arg)
{
	struct thread_args *args;
	char key_buf[64];
	char value_buf[128];
	int rc;
	int i;
	int local_errors;

	args = (struct thread_args *)arg;
	local_errors = 0;

	for (i = 0; i < OPS_PER_THREAD * 2; i++) {
		snprintf(key_buf, sizeof(key_buf), "resize_key_%d_%d",
			 args->thread_id, i);
		snprintf(value_buf, sizeof(value_buf),
			 "resize_value_%d_%d_with_padding", args->thread_id, i);
		rc = hash_put(args->engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0 && rc != -ENOMEM && rc != -ENOSPC) {
			local_errors++;
		}
	}

	pthread_mutex_lock(args->error_mutex);
	*args->error_count += local_errors;
	pthread_mutex_unlock(args->error_mutex);

	return NULL;
}

static int
test_concurrent_resize(void)
{
	struct hash_engine engine;
	pthread_t threads[NUM_THREADS];
	struct thread_args args[NUM_THREADS];
	pthread_mutex_t error_mutex;
	int error_count;
	uint32_t initial_buckets;
	uint32_t final_buckets;
	int rc;
	int i;

	error_count = 0;

	/* Start with small bucket count to force resizes */
	rc = hash_engine_init(&engine, INITIAL_BUCKET_COUNT);
	if (rc != 0) {
		return TEST_FAILED;
	}

	rc = hash_engine_get_stats(&engine, NULL, &initial_buckets, NULL);
	if (rc != 0) {
		hash_engine_destroy(&engine);
		return TEST_FAILED;
	}

	pthread_mutex_init(&error_mutex, NULL);

	/* Launch threads that will trigger resizes */
	for (i = 0; i < NUM_THREADS; i++) {
		args[i].engine = &engine;
		args[i].thread_id = i;
		args[i].error_count = &error_count;
		args[i].error_mutex = &error_mutex;
		pthread_create(&threads[i], NULL, concurrent_resize_worker,
			       &args[i]);
	}

	/* Wait for completion */
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	rc = hash_engine_get_stats(&engine, NULL, &final_buckets, NULL);

	pthread_mutex_destroy(&error_mutex);
	hash_engine_destroy(&engine);

	if (error_count > 0) {
		fprintf(stderr, "Concurrent resize had %d errors\n",
			error_count);
		return TEST_FAILED;
	}

	if (final_buckets <= initial_buckets) {
		fprintf(stderr, "Resize did not occur: initial=%u final=%u\n",
			initial_buckets, final_buckets);
		return TEST_FAILED;
	}

	return TEST_PASSED;
}

int
main(void)
{
	printf("===== Hash Table Concurrency Tests =====\n");
	printf("Testing with %d threads, %d ops/thread\n\n", NUM_THREADS,
	       OPS_PER_THREAD);

	RUN_TEST(test_concurrent_reads);
	RUN_TEST(test_concurrent_writes_unique_keys);
	RUN_TEST(test_concurrent_writes_same_key);
	RUN_TEST(test_concurrent_mixed_operations);
	RUN_TEST(test_concurrent_resize);

	printf("\n========================================\n");
	printf("Tests run: %d\n", tests_run);
	printf("Tests passed: %d\n", tests_passed);
	printf("Tests failed: %d\n", tests_failed);
	printf("========================================\n");
	printf("\nNOTE: Run with ThreadSanitizer:\n");
	printf("  gcc -fsanitize=thread -g -O1 ...\n");

	return tests_failed > 0 ? 1 : 0;
}
