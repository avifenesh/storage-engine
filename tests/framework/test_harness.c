/**
 * @file test_harness.c
 * @brief Implementation of test harness utilities
 */

#include "test_harness.h"
#include <sys/time.h>

void
test_harness_init(struct test_context *ctx, bool verbose, bool use_colors)
{
	if (!ctx)
		return;

	ctx->stats.tests_run = 0;
	ctx->stats.tests_passed = 0;
	ctx->stats.tests_failed = 0;
	ctx->stats.tests_skipped = 0;
	ctx->stats.total_time_usec = 0;
	ctx->current_test_name = NULL;
	ctx->test_start_time = 0;
	ctx->verbose = verbose;
	ctx->use_colors = use_colors;
}

void
test_harness_print_summary(struct test_context *ctx)
{
	double pass_rate;

	if (!ctx)
		return;

	printf("\n========================================\n");

	if (ctx->stats.tests_run > 0) {
		pass_rate = (100.0 * ctx->stats.tests_passed)
			    / ctx->stats.tests_run;
	} else {
		pass_rate = 0.0;
	}

	printf("Tests run:     %s%d%s\n", ctx->use_colors ? COLOR_CYAN : "",
	       ctx->stats.tests_run, ctx->use_colors ? COLOR_RESET : "");
	printf("Tests passed:  %s%d%s (%.1f%%)\n",
	       ctx->use_colors ? COLOR_GREEN : "", ctx->stats.tests_passed,
	       ctx->use_colors ? COLOR_RESET : "", pass_rate);

	if (ctx->stats.tests_failed > 0) {
		printf("Tests failed:  %s%d%s\n",
		       ctx->use_colors ? COLOR_RED : "",
		       ctx->stats.tests_failed,
		       ctx->use_colors ? COLOR_RESET : "");
	}

	if (ctx->stats.tests_skipped > 0) {
		printf("Tests skipped: %s%d%s\n",
		       ctx->use_colors ? COLOR_YELLOW : "",
		       ctx->stats.tests_skipped,
		       ctx->use_colors ? COLOR_RESET : "");
	}

	printf("Total time:    %.3f seconds\n",
	       ctx->stats.total_time_usec / 1000000.0);
	printf("========================================\n");
}

long long
test_get_time_usec(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long long)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}

double
test_get_time_sec(void)
{
	return test_get_time_usec() / 1000000.0;
}

void
test_gen_random_bytes(void *buffer, size_t size)
{
	unsigned char *buf;
	size_t i;

	buf = (unsigned char *)buffer;
	for (i = 0; i < size; i++) {
		buf[i] = (unsigned char)(rand() % 256);
	}
}

void
test_gen_random_string(char *buffer, size_t size)
{
	const char charset[]
	    = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	size_t i;

	if (size == 0)
		return;

	for (i = 0; i < size - 1; i++) {
		buffer[i] = charset[rand() % (sizeof(charset) - 1)];
	}
	buffer[size - 1] = '\0';
}

int
test_gen_random_int(int min, int max)
{
	return min + (rand() % (max - min + 1));
}

void
test_fill_pattern_sequential(void *buffer, size_t size)
{
	unsigned char *buf;
	size_t i;

	buf = (unsigned char *)buffer;
	for (i = 0; i < size; i++) {
		buf[i] = (unsigned char)(i % 256);
	}
}

void
test_fill_pattern_zeros(void *buffer, size_t size)
{
	memset(buffer, 0, size);
}

void
test_fill_pattern_ones(void *buffer, size_t size)
{
	memset(buffer, 0xFF, size);
}

void
test_fill_pattern_alternating(void *buffer, size_t size)
{
	unsigned char *buf;
	size_t i;

	buf = (unsigned char *)buffer;
	for (i = 0; i < size; i++) {
		buf[i] = (i % 2) ? 0xFF : 0x00;
	}
}

char *
test_format_bytes(size_t bytes)
{
	static char buffer[64];
	const char *units[] = { "B", "KB", "MB", "GB" };
	double size;
	int unit;

	size = (double)bytes;
	unit = 0;

	while (size >= 1024.0 && unit < 3) {
		size /= 1024.0;
		unit++;
	}

	snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unit]);
	return buffer;
}

char *
test_format_duration(long long usec)
{
	static char buffer[64];

	if (usec < 1000) {
		snprintf(buffer, sizeof(buffer), "%lld µs", usec);
	} else if (usec < 1000000) {
		snprintf(buffer, sizeof(buffer), "%.3f ms", usec / 1000.0);
	} else {
		snprintf(buffer, sizeof(buffer), "%.3f s", usec / 1000000.0);
	}

	return buffer;
}

bool
test_verify_pattern_sequential(const void *buffer, size_t size)
{
	const unsigned char *buf;
	size_t i;

	buf = (const unsigned char *)buffer;
	for (i = 0; i < size; i++) {
		if (buf[i] != (unsigned char)(i % 256)) {
			return false;
		}
	}
	return true;
}

bool
test_verify_pattern_zeros(const void *buffer, size_t size)
{
	const unsigned char *buf;
	size_t i;

	buf = (const unsigned char *)buffer;
	for (i = 0; i < size; i++) {
		if (buf[i] != 0) {
			return false;
		}
	}
	return true;
}
