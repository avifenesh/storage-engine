/**
 * @file test_harness.h
 * @brief Common test harness utilities and helpers
 *
 * Provides shared functionality for all test suites including:
 * - Test result tracking and reporting
 * - Common assertions and validation
 * - Test data generation
 * - Memory tracking
 */

#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Test result codes */
#define TEST_PASSED 0
#define TEST_FAILED 1
#define TEST_SKIPPED 2

/* Color codes for terminal output */
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"

/* Test statistics */
struct test_stats {
	int tests_run;
	int tests_passed;
	int tests_failed;
	int tests_skipped;
	long long total_time_usec;
};

/* Test context for tracking state */
struct test_context {
	struct test_stats stats;
	const char *current_test_name;
	long long test_start_time;
	bool verbose;
	bool use_colors;
};

/* Initialize test context */
void test_harness_init(struct test_context *ctx, bool verbose, bool use_colors);

/* Print test results summary */
void test_harness_print_summary(struct test_context *ctx);

/* Test execution macro with timing */
#define RUN_TEST_CTX(ctx, test)                                               \
	do {                                                                  \
		(ctx)->current_test_name = #test;                             \
		(ctx)->test_start_time = test_get_time_usec();               \
		printf("%sRunning %s...%s", (ctx)->use_colors ? COLOR_CYAN   \
						: "",                         \
		       #test, (ctx)->use_colors ? COLOR_RESET : "");          \
		fflush(stdout);                                               \
		(ctx)->stats.tests_run++;                                     \
		int result = (test)();                                        \
		long long elapsed = test_get_time_usec()                      \
				    - (ctx)->test_start_time;                 \
		(ctx)->stats.total_time_usec += elapsed;                      \
		if (result == TEST_PASSED) {                                  \
			printf(" %sPASSED%s (%.3f ms)\n",                     \
			       (ctx)->use_colors ? COLOR_GREEN : "",          \
			       (ctx)->use_colors ? COLOR_RESET : "",          \
			       elapsed / 1000.0);                             \
			(ctx)->stats.tests_passed++;                          \
		} else if (result == TEST_SKIPPED) {                          \
			printf(" %sSKIPPED%s\n",                              \
			       (ctx)->use_colors ? COLOR_YELLOW : "",         \
			       (ctx)->use_colors ? COLOR_RESET : "");         \
			(ctx)->stats.tests_skipped++;                         \
		} else {                                                      \
			printf(" %sFAILED%s\n",                               \
			       (ctx)->use_colors ? COLOR_RED : "",            \
			       (ctx)->use_colors ? COLOR_RESET : "");         \
			(ctx)->stats.tests_failed++;                          \
		}                                                             \
	} while (0)

/* Assertion helpers */
#define ASSERT_EQ(actual, expected, msg)                                      \
	do {                                                                  \
		if ((actual) != (expected)) {                                 \
			fprintf(stderr,                                       \
				"  Assertion failed: %s\n"                    \
				"    Expected: %d\n"                          \
				"    Actual: %d\n",                           \
				(msg), (int)(expected), (int)(actual));       \
			return TEST_FAILED;                                   \
		}                                                             \
	} while (0)

#define ASSERT_NE(actual, unexpected, msg)                                    \
	do {                                                                  \
		if ((actual) == (unexpected)) {                               \
			fprintf(stderr,                                       \
				"  Assertion failed: %s\n"                    \
				"    Value should not be: %d\n",              \
				(msg), (int)(unexpected));                    \
			return TEST_FAILED;                                   \
		}                                                             \
	} while (0)

#define ASSERT_TRUE(condition, msg)                                           \
	do {                                                                  \
		if (!(condition)) {                                           \
			fprintf(stderr, "  Assertion failed: %s\n", (msg));   \
			return TEST_FAILED;                                   \
		}                                                             \
	} while (0)

#define ASSERT_FALSE(condition, msg)                                          \
	do {                                                                  \
		if (condition) {                                              \
			fprintf(stderr, "  Assertion failed: %s\n", (msg));   \
			return TEST_FAILED;                                   \
		}                                                             \
	} while (0)

#define ASSERT_NULL(ptr, msg)                                                 \
	do {                                                                  \
		if ((ptr) != NULL) {                                          \
			fprintf(stderr,                                       \
				"  Assertion failed: %s\n"                    \
				"    Pointer should be NULL\n",               \
				(msg));                                       \
			return TEST_FAILED;                                   \
		}                                                             \
	} while (0)

#define ASSERT_NOT_NULL(ptr, msg)                                             \
	do {                                                                  \
		if ((ptr) == NULL) {                                          \
			fprintf(stderr,                                       \
				"  Assertion failed: %s\n"                    \
				"    Pointer should not be NULL\n",           \
				(msg));                                       \
			return TEST_FAILED;                                   \
		}                                                             \
	} while (0)

#define ASSERT_MEM_EQ(actual, expected, size, msg)                            \
	do {                                                                  \
		if (memcmp((actual), (expected), (size)) != 0) {              \
			fprintf(stderr,                                       \
				"  Assertion failed: %s\n"                    \
				"    Memory contents differ\n",               \
				(msg));                                       \
			return TEST_FAILED;                                   \
		}                                                             \
	} while (0)

/* Time measurement helpers */
long long test_get_time_usec(void);
double test_get_time_sec(void);

/* Random data generation */
void test_gen_random_bytes(void *buffer, size_t size);
void test_gen_random_string(char *buffer, size_t size);
int test_gen_random_int(int min, int max);

/* Test data patterns */
void test_fill_pattern_sequential(void *buffer, size_t size);
void test_fill_pattern_zeros(void *buffer, size_t size);
void test_fill_pattern_ones(void *buffer, size_t size);
void test_fill_pattern_alternating(void *buffer, size_t size);

/* String helpers */
char *test_format_bytes(size_t bytes);
char *test_format_duration(long long usec);

/* Memory verification helpers */
bool test_verify_pattern_sequential(const void *buffer, size_t size);
bool test_verify_pattern_zeros(const void *buffer, size_t size);

#endif /* TEST_HARNESS_H */
