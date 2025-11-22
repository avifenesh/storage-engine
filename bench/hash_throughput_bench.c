/**
 * @file hash_throughput_bench.c
 * @brief Throughput benchmarks for hash table
 *
 * Measures operations per second for different workloads and configurations.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "storage/hash_engine.h"

#define MILLION 1000000
#define THOUSAND 1000

/* Helper to get time in microseconds */
static long long
get_time_usec(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long long)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}

/* Benchmark: Insert throughput */
static void
bench_insert_throughput(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char value_buf[128];
	long long start;
	long long end;
	double elapsed_sec;
	double ops_per_sec;
	int rc;
	int i;
	const int NUM_OPS = 100000;

	printf("Benchmarking INSERT throughput (%d operations)...\n", NUM_OPS);

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "Init failed\n");
		return;
	}

	start = get_time_usec();

	for (i = 0; i < NUM_OPS; i++) {
		snprintf(key_buf, sizeof(key_buf), "bench_insert_key_%d", i);
		snprintf(value_buf, sizeof(value_buf), "bench_insert_value_%d",
			 i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr, "Insert failed at op %d\n", i);
			break;
		}
	}

	end = get_time_usec();

	elapsed_sec = (end - start) / 1000000.0;
	ops_per_sec = NUM_OPS / elapsed_sec;

	printf("  Time: %.3f seconds\n", elapsed_sec);
	printf("  Throughput: %.0f ops/sec\n", ops_per_sec);
	printf("  Latency: %.3f µs/op\n\n", (elapsed_sec * 1000000) / NUM_OPS);

	hash_engine_destroy(&engine);
}

/* Benchmark: Get throughput (after population) */
static void
bench_get_throughput(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char value_buf[128];
	const void *retrieved_value;
	size_t retrieved_len;
	long long start;
	long long end;
	double elapsed_sec;
	double ops_per_sec;
	int rc;
	int i;
	const int NUM_OPS = 100000;

	printf("Benchmarking GET throughput (%d operations)...\n", NUM_OPS);

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "Init failed\n");
		return;
	}

	/* Populate */
	for (i = 0; i < NUM_OPS; i++) {
		snprintf(key_buf, sizeof(key_buf), "bench_get_key_%d", i);
		snprintf(value_buf, sizeof(value_buf), "bench_get_value_%d", i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr, "Setup insert failed\n");
			hash_engine_destroy(&engine);
			return;
		}
	}

	/* Benchmark gets */
	start = get_time_usec();

	for (i = 0; i < NUM_OPS; i++) {
		snprintf(key_buf, sizeof(key_buf), "bench_get_key_%d", i);
		rc = hash_get(&engine, key_buf, strlen(key_buf),
			      &retrieved_value, &retrieved_len);
		if (rc != 0) {
			fprintf(stderr, "Get failed at op %d\n", i);
			break;
		}
	}

	end = get_time_usec();

	elapsed_sec = (end - start) / 1000000.0;
	ops_per_sec = NUM_OPS / elapsed_sec;

	printf("  Time: %.3f seconds\n", elapsed_sec);
	printf("  Throughput: %.0f ops/sec\n", ops_per_sec);
	printf("  Latency: %.3f µs/op\n\n", (elapsed_sec * 1000000) / NUM_OPS);

	hash_engine_destroy(&engine);
}

/* Benchmark: Delete throughput */
static void
bench_delete_throughput(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char value_buf[128];
	long long start;
	long long end;
	double elapsed_sec;
	double ops_per_sec;
	int rc;
	int i;
	const int NUM_OPS = 100000;

	printf("Benchmarking DELETE throughput (%d operations)...\n", NUM_OPS);

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "Init failed\n");
		return;
	}

	/* Populate */
	for (i = 0; i < NUM_OPS; i++) {
		snprintf(key_buf, sizeof(key_buf), "bench_delete_key_%d", i);
		snprintf(value_buf, sizeof(value_buf), "bench_delete_value_%d",
			 i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr, "Setup insert failed\n");
			hash_engine_destroy(&engine);
			return;
		}
	}

	/* Benchmark deletes */
	start = get_time_usec();

	for (i = 0; i < NUM_OPS; i++) {
		snprintf(key_buf, sizeof(key_buf), "bench_delete_key_%d", i);
		rc = hash_delete(&engine, key_buf, strlen(key_buf));
		if (rc != 0) {
			fprintf(stderr, "Delete failed at op %d\n", i);
			break;
		}
	}

	end = get_time_usec();

	elapsed_sec = (end - start) / 1000000.0;
	ops_per_sec = NUM_OPS / elapsed_sec;

	printf("  Time: %.3f seconds\n", elapsed_sec);
	printf("  Throughput: %.0f ops/sec\n", ops_per_sec);
	printf("  Latency: %.3f µs/op\n\n", (elapsed_sec * 1000000) / NUM_OPS);

	hash_engine_destroy(&engine);
}

/* Benchmark: Mixed workload (50% read, 25% write, 25% delete) */
static void
bench_mixed_workload(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char value_buf[128];
	const void *retrieved_value;
	size_t retrieved_len;
	long long start;
	long long end;
	double elapsed_sec;
	double ops_per_sec;
	int rc;
	int i;
	int operation;
	int key_space;
	const int NUM_OPS = 100000;

	printf("Benchmarking MIXED workload (%d operations)...\n", NUM_OPS);
	printf("  50%% reads, 25%% writes, 25%% deletes\n");

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "Init failed\n");
		return;
	}

	/* Pre-populate half the key space */
	key_space = 50000;
	for (i = 0; i < key_space / 2; i++) {
		snprintf(key_buf, sizeof(key_buf), "bench_mixed_key_%d", i);
		snprintf(value_buf, sizeof(value_buf), "bench_mixed_value_%d",
			 i);
		rc = hash_put(&engine, key_buf, strlen(key_buf), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr, "Setup insert failed\n");
			hash_engine_destroy(&engine);
			return;
		}
	}

	srand(42); /* Fixed seed for reproducibility */

	start = get_time_usec();

	for (i = 0; i < NUM_OPS; i++) {
		int key_id = rand() % key_space;
		operation = rand() % 4; /* 0-1: read, 2: write, 3: delete */

		snprintf(key_buf, sizeof(key_buf), "bench_mixed_key_%d",
			 key_id);
		snprintf(value_buf, sizeof(value_buf), "bench_mixed_value_%d",
			 key_id);

		if (operation <= 1) { /* 50% reads */
			rc = hash_get(&engine, key_buf, strlen(key_buf),
				      &retrieved_value, &retrieved_len);
		} else if (operation == 2) { /* 25% writes */
			rc = hash_put(&engine, key_buf, strlen(key_buf),
				      value_buf, strlen(value_buf));
		} else { /* 25% deletes */
			rc = hash_delete(&engine, key_buf, strlen(key_buf));
		}
	}

	end = get_time_usec();

	elapsed_sec = (end - start) / 1000000.0;
	ops_per_sec = NUM_OPS / elapsed_sec;

	printf("  Time: %.3f seconds\n", elapsed_sec);
	printf("  Throughput: %.0f ops/sec\n", ops_per_sec);
	printf("  Latency: %.3f µs/op\n\n", (elapsed_sec * 1000000) / NUM_OPS);

	hash_engine_destroy(&engine);
}

/* Benchmark: Update throughput */
static void
bench_update_throughput(void)
{
	struct hash_engine engine;
	const char *key = "bench_update_key";
	char value_buf[128];
	long long start;
	long long end;
	double elapsed_sec;
	double ops_per_sec;
	int rc;
	int i;
	const int NUM_OPS = 100000;

	printf("Benchmarking UPDATE throughput (%d operations)...\n", NUM_OPS);
	printf("  Updating single key repeatedly\n");

	rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
	if (rc != 0) {
		fprintf(stderr, "Init failed\n");
		return;
	}

	/* Initial insert */
	snprintf(value_buf, sizeof(value_buf), "initial_value");
	rc = hash_put(&engine, key, strlen(key), value_buf, strlen(value_buf));
	if (rc != 0) {
		fprintf(stderr, "Initial insert failed\n");
		hash_engine_destroy(&engine);
		return;
	}

	start = get_time_usec();

	for (i = 0; i < NUM_OPS; i++) {
		snprintf(value_buf, sizeof(value_buf), "updated_value_%d", i);
		rc = hash_put(&engine, key, strlen(key), value_buf,
			      strlen(value_buf));
		if (rc != 0) {
			fprintf(stderr, "Update failed at op %d\n", i);
			break;
		}
	}

	end = get_time_usec();

	elapsed_sec = (end - start) / 1000000.0;
	ops_per_sec = NUM_OPS / elapsed_sec;

	printf("  Time: %.3f seconds\n", elapsed_sec);
	printf("  Throughput: %.0f ops/sec\n", ops_per_sec);
	printf("  Latency: %.3f µs/op\n\n", (elapsed_sec * 1000000) / NUM_OPS);

	hash_engine_destroy(&engine);
}

/* Benchmark: Varying value sizes */
static void
bench_varying_value_sizes(void)
{
	size_t sizes[] = { 8, 64, 256, 1024, 4096, 16384 };
	int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
	struct hash_engine engine;
	char key_buf[64];
	char *value_buf;
	long long start;
	long long end;
	double elapsed_sec;
	double ops_per_sec;
	int rc;
	int i;
	int j;
	const int NUM_OPS = 10000;

	printf("Benchmarking INSERT with varying value sizes...\n");

	for (j = 0; j < num_sizes; j++) {
		size_t value_size = sizes[j];

		printf("  Value size: %zu bytes\n", value_size);

		rc = hash_engine_init(&engine, DEFAULT_BUCKET_COUNT);
		if (rc != 0) {
			fprintf(stderr, "Init failed\n");
			continue;
		}

		value_buf = malloc(value_size);
		if (!value_buf) {
			fprintf(stderr, "Malloc failed\n");
			hash_engine_destroy(&engine);
			continue;
		}
		memset(value_buf, 'V', value_size);

		start = get_time_usec();

		for (i = 0; i < NUM_OPS; i++) {
			snprintf(key_buf, sizeof(key_buf), "varsize_key_%d", i);
			rc = hash_put(&engine, key_buf, strlen(key_buf),
				      value_buf, value_size);
			if (rc != 0) {
				fprintf(stderr, "Insert failed at op %d\n", i);
				break;
			}
		}

		end = get_time_usec();

		elapsed_sec = (end - start) / 1000000.0;
		ops_per_sec = NUM_OPS / elapsed_sec;

		printf("    Throughput: %.0f ops/sec\n", ops_per_sec);
		printf("    Latency: %.3f µs/op\n",
		       (elapsed_sec * 1000000) / NUM_OPS);

		free(value_buf);
		hash_engine_destroy(&engine);
	}
	printf("\n");
}

/* Benchmark: Load factor impact */
static void
bench_load_factor_impact(void)
{
	struct hash_engine engine;
	char key_buf[64];
	char value_buf[128];
	const void *retrieved_value;
	size_t retrieved_len;
	uint32_t bucket_count;
	long long start;
	long long end;
	double elapsed_sec;
	double ops_per_sec;
	float load_factor;
	int rc;
	int i;
	int num_items;
	const int BASE_BUCKETS = 1024;
	const int NUM_LOOKUPS = 10000;

	printf("Benchmarking GET performance vs load factor...\n");

	for (num_items = 128; num_items <= BASE_BUCKETS * 2; num_items *= 2) {
		rc = hash_engine_init(&engine, BASE_BUCKETS);
		if (rc != 0) {
			fprintf(stderr, "Init failed\n");
			continue;
		}

		/* Populate */
		for (i = 0; i < num_items; i++) {
			snprintf(key_buf, sizeof(key_buf), "loadfactor_key_%d",
				 i);
			snprintf(value_buf, sizeof(value_buf),
				 "loadfactor_value_%d", i);
			rc = hash_put(&engine, key_buf, strlen(key_buf),
				      value_buf, strlen(value_buf));
			if (rc != 0)
				break;
		}

		rc = hash_engine_get_stats(&engine, NULL, &bucket_count, NULL);
		if (rc != 0) {
			hash_engine_destroy(&engine);
			continue;
		}

		load_factor = (float)num_items / (float)bucket_count;

		/* Benchmark lookups */
		start = get_time_usec();

		for (i = 0; i < NUM_LOOKUPS; i++) {
			int key_id = i % num_items;
			snprintf(key_buf, sizeof(key_buf), "loadfactor_key_%d",
				 key_id);
			rc = hash_get(&engine, key_buf, strlen(key_buf),
				      &retrieved_value, &retrieved_len);
		}

		end = get_time_usec();

		elapsed_sec = (end - start) / 1000000.0;
		ops_per_sec = NUM_LOOKUPS / elapsed_sec;

		printf("  Load factor: %.2f (items=%d, buckets=%u)\n",
		       load_factor, num_items, bucket_count);
		printf("    GET throughput: %.0f ops/sec\n", ops_per_sec);

		hash_engine_destroy(&engine);
	}
	printf("\n");
}

int
main(void)
{
	printf("===== Hash Table Throughput Benchmarks =====\n\n");

	bench_insert_throughput();
	bench_get_throughput();
	bench_delete_throughput();
	bench_update_throughput();
	bench_mixed_workload();
	bench_varying_value_sizes();
	bench_load_factor_impact();

	printf("========================================\n");
	printf("Benchmarks complete\n");

	return 0;
}
