/**
 * @file hash_chaos_test.c
 * @brief Chaos testing for hash table
 *
 * Introduces random failures, timing variations, and unpredictable
 * sequences to expose edge cases and race conditions.
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

#define NUM_CHAOS_THREADS 4
#define CHAOS_DURATION_SEC 10
#define MAX_CHAOS_OPS 10000

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

/* Chaos testing state */
struct chaos_state {
	struct hash_engine *engine;
	volatile bool stop;
	int thread_id;
	int ops_performed;
	int ops_succeeded;
	int ops_failed;
	pthread_mutex_t *stats_mutex;
};

/* Random delay to introduce timing variations */
static void
chaos_random_delay(void)
{
	int delay_type = rand() % 10;

	if (delay_type < 7) {
		/* No delay - 70% */
		return;
	} else if (delay_type < 9) {
		/* Short delay - 20% */
		usleep(rand() % 100);
	} else {
		/* Long delay - 10% */
		usleep(rand() % 1000);
	}
}

/* Chaos worker thread */
static void *
chaos_worker(void *arg)
{
	struct chaos_state *state;
	char key_buf[64];
	char value_buf[128];
	const void *retrieved_value;
	size_t retrieved_len;
	int operation;
	int key_id;
	int rc;

	state = (struct chaos_state *)arg;

	/* Thread-local random seed */
	srand((unsigned int)time(NULL) + state->thread_id);

	while (!state->stop && state->ops_performed < MAX_CHAOS_OPS) {
		/* Random operation */
		operation = rand() % 100;
		key_id = rand() % 500; /* Limited key space for collisions */

		snprintf(key_buf, sizeof(key_buf), "chaos_key_%d", key_id);
		snprintf(value_buf, sizeof(value_buf), "chaos_value_%d_%d",
			 state->thread_id, state->ops_performed);

		/* Introduce random delays */
		chaos_random_delay();

		if (operation < 50) { /* 50% PUT */
			rc = hash_put(state->engine, key_buf, strlen(key_buf),
				      value_buf, strlen(value_buf));
		} else if (operation < 80) { /* 30% GET */
			rc = hash_get(state->engine, key_buf, strlen(key_buf),
				      &retrieved_value, &retrieved_len);
		} else if (operation < 95) { /* 15% DELETE */
			rc = hash_delete(state->engine, key_buf,
					 strlen(key_buf));
		} else { /* 5% GET_STATS */
			uint32_t item_count;
			uint32_t bucket_count;
			uint32_t memory_usage;
			rc = hash_engine_get_stats(state->engine, &item_count,
						   &bucket_count,
						   &memory_usage);
		}

		state->ops_performed++;
		if (rc == 0) {
			state->ops_succeeded++;
		} else {
			state->ops_failed++;
		}

		/* Occasional random sleep */
		if (rand() % 100 < 5) {
			usleep(rand() % 1000);
		}
	}

	return NULL;
}

/* Test: Chaos testing with random operations and timing */
static int
test_chaos_random_operations(void)
{
	struct hash_engine engine;
	pthread_t threads[NUM_CHAOS_THREADS];
	struct chaos_state states[NUM_CHAOS_THREADS];
	pthread_mutex_t stats_mutex;
	time_t start_time;
	time_t current_time;
	int total_ops;
	int total_succeeded;
	int total_failed;
	int rc;
	int i;

	printf("\n  Running chaos test for %d seconds with %d threads...\n",
	       CHAOS_DURATION_SEC, NUM_CHAOS_THREADS);

	rc = hash_engine_init(&engine, 128);
	if (rc != 0) {
		fprintf(stderr, "  Failed to initialize engine\n");
		return TEST_FAILED;
	}

	pthread_mutex_init(&stats_mutex, NULL);

	/* Launch chaos threads */
	start_time = time(NULL);
	for (i = 0; i < NUM_CHAOS_THREADS; i++) {
		states[i].engine = &engine;
		states[i].stop = false;
		states[i].thread_id = i;
		states[i].ops_performed = 0;
		states[i].ops_succeeded = 0;
		states[i].ops_failed = 0;
		states[i].stats_mutex = &stats_mutex;
		pthread_create(&threads[i], NULL, chaos_worker, &states[i]);
	}

	/* Run for specified duration */
	sleep(CHAOS_DURATION_SEC);

	/* Stop all threads */
	for (i = 0; i < NUM_CHAOS_THREADS; i++) {
		states[i].stop = true;
	}

	/* Wait for completion */
	for (i = 0; i < NUM_CHAOS_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	current_time = time(NULL);

	/* Collect statistics */
	total_ops = 0;
	total_succeeded = 0;
	total_failed = 0;
	for (i = 0; i < NUM_CHAOS_THREADS; i++) {
		total_ops += states[i].ops_performed;
		total_succeeded += states[i].ops_succeeded;
		total_failed += states[i].ops_failed;
	}

	printf("  Duration: %ld seconds\n", (long)(current_time - start_time));
	printf("  Total operations: %d\n", total_ops);
	printf("  Succeeded: %d (%.1f%%)\n", total_succeeded,
	       100.0 * total_succeeded / total_ops);
	printf("  Failed: %d (%.1f%%)\n", total_failed,
	       100.0 * total_failed / total_ops);
	printf("  Ops/sec: %.0f\n",
	       total_ops / (double)(current_time - start_time));

	pthread_mutex_destroy(&stats_mutex);
	hash_engine_destroy(&engine);

	/* Test passes if we survived without crashes */
	return TEST_PASSED;
}

/* Test: Chaos with rapid init/destroy cycles */
static int
test_chaos_init_destroy(void)
{
	struct hash_engine engine;
	int i;
	int rc;
	const int NUM_CYCLES = 1000;

	printf("\n  Performing %d rapid init/destroy cycles...\n", NUM_CYCLES);

	for (i = 0; i < NUM_CYCLES; i++) {
		int bucket_count;
		int num_ops;
		int j;
		char key[32];
		char value[64];

		/* Random bucket count */
		bucket_count = 16 << (rand() % 6); /* 16, 32, 64, 128, 256, 512 */

		rc = hash_engine_init(&engine, bucket_count);
		if (rc != 0) {
			fprintf(stderr, "  Init failed at cycle %d\n", i);
			return TEST_FAILED;
		}

		/* Random number of operations */
		num_ops = rand() % 20;
		for (j = 0; j < num_ops; j++) {
			snprintf(key, sizeof(key), "key_%d", j);
			snprintf(value, sizeof(value), "value_%d", j);
			(void)hash_put(&engine, key, strlen(key), value,
				       strlen(value));
		}

		rc = hash_engine_destroy(&engine);
		if (rc != 0) {
			fprintf(stderr, "  Destroy failed at cycle %d\n", i);
			return TEST_FAILED;
		}

		/* Random delay */
		if (rand() % 10 < 2) {
			usleep(rand() % 100);
		}
	}

	printf("  Completed %d cycles\n", NUM_CYCLES);
	return TEST_PASSED;
}

/* Test: Chaos with interleaved operations */
static int
test_chaos_interleaved_ops(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char value_buf[128];
	const void *retrieved_value;
	size_t retrieved_len;
	int i;
	int rc;
	const int NUM_OPS = 5000;

	printf("\n  Performing %d interleaved operations...\n", NUM_OPS);

	rc = hash_engine_init(&engine, 64);
	if (rc != 0) {
		return TEST_FAILED;
	}

	srand(42); /* Fixed seed for reproducibility */

	for (i = 0; i < NUM_OPS; i++) {
		int key_id = rand() % 100;
		int op_type = rand() % 4;

		snprintf(key_buf, sizeof(key_buf), "interleaved_key_%d",
			 key_id);
		snprintf(value_buf, sizeof(value_buf), "interleaved_val_%d", i);

		switch (op_type) {
		case 0:
		case 1: /* PUT (50%) */
			(void)hash_put(&engine, key_buf, strlen(key_buf),
				       value_buf, strlen(value_buf));
			break;
		case 2: /* GET (25%) */
			(void)hash_get(&engine, key_buf, strlen(key_buf),
				       &retrieved_value, &retrieved_len);
			break;
		case 3: /* DELETE (25%) */
			(void)hash_delete(&engine, key_buf, strlen(key_buf));
			break;
		}

		/* Verify consistency periodically */
		if (i % 500 == 0) {
			uint32_t item_count;
			rc = hash_engine_get_stats(&engine, &item_count, NULL,
						   NULL);
			if (rc != 0) {
				fprintf(stderr,
					"  Stats failed at operation %d\n", i);
				hash_engine_destroy(&engine);
				return TEST_FAILED;
			}
		}
	}

	printf("  Completed %d operations\n", NUM_OPS);
	hash_engine_destroy(&engine);
	return TEST_PASSED;
}

int
main(void)
{
	printf("===== Hash Table Chaos Tests =====\n");
	printf("Testing with random operations, timing variations, and "
	       "unpredictable sequences\n\n");

	RUN_TEST(test_chaos_random_operations);
	RUN_TEST(test_chaos_init_destroy);
	RUN_TEST(test_chaos_interleaved_ops);

	printf("\n========================================\n");
	printf("Tests run: %d\n", tests_run);
	printf("Tests passed: %d\n", tests_passed);
	printf("Tests failed: %d\n", tests_failed);
	printf("========================================\n");

	return tests_failed > 0 ? 1 : 0;
}
