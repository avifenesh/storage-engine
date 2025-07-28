/*
 * Hash Storage Engine - Unit Tests
 * 
 * Test suite for hash table operations, NEON optimization,
 * and character device interface.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

/* Test framework macros */
#define TEST_ASSERT(condition, message) \
	do { \
		if (!(condition)) { \
			fprintf(stderr, "FAIL: %s\n", message); \
			test_failures++; \
		} else { \
			printf("PASS: %s\n", message); \
		} \
		test_count++; \
	} while(0)

#define TEST_START(name) \
	printf("\n=== Testing %s ===\n", name); \
	int test_count = 0, test_failures = 0;

#define TEST_SUMMARY() \
	printf("Tests: %d, Passed: %d, Failed: %d\n", \
		test_count, test_count - test_failures, test_failures); \
	return test_failures;

/* IOCTL definitions (must match kernel module) */
#define HASH_IOC_MAGIC 'H'
#define HASH_IOC_PUT    _IOW(HASH_IOC_MAGIC, 1, struct hash_operation)
#define HASH_IOC_GET    _IOWR(HASH_IOC_MAGIC, 2, struct hash_operation)
#define HASH_IOC_DELETE _IOW(HASH_IOC_MAGIC, 3, struct hash_operation)
#define HASH_IOC_STATS  _IOR(HASH_IOC_MAGIC, 4, struct hash_stats)

struct hash_operation {
	void *key;
	size_t key_len;
	void *value;
	size_t value_len;
};

struct hash_stats {
	unsigned int item_count;
	unsigned int bucket_count;
	unsigned int memory_usage;
};

/*
 * Test basic hash operations via device interface
 */
int test_basic_operations(void)
{
	TEST_START("Basic Hash Operations");
	
	int fd = open("/dev/storage-hash", O_RDWR);
	if (fd < 0) {
		printf("ERROR: Cannot open /dev/storage-hash: %s\n", strerror(errno));
		printf("Make sure the kernel module is loaded.\n");
		return 1;
	}

	/* Test data */
	char key1[] = "test_key_1";
	char value1[] = "test_value_1";
	char key2[] = "test_key_2";
	char value2[] = "test_value_2_longer";
	char retrieved_value[256];

	/* Test PUT operation */
	struct hash_operation put_op = {
		.key = key1,
		.key_len = strlen(key1),
		.value = value1,
		.value_len = strlen(value1)
	};
	
	int result = ioctl(fd, HASH_IOC_PUT, &put_op);
	TEST_ASSERT(result == 0, "PUT operation successful");

	/* Test GET operation */
	struct hash_operation get_op = {
		.key = key1,
		.key_len = strlen(key1),
		.value = retrieved_value,
		.value_len = sizeof(retrieved_value)
	};
	
	result = ioctl(fd, HASH_IOC_GET, &get_op);
	TEST_ASSERT(result == 0, "GET operation successful");
	TEST_ASSERT(get_op.value_len == strlen(value1), "Retrieved value length correct");
	TEST_ASSERT(memcmp(retrieved_value, value1, get_op.value_len) == 0, "Retrieved value content correct");

	/* Test PUT second key */
	put_op.key = key2;
	put_op.key_len = strlen(key2);
	put_op.value = value2;
	put_op.value_len = strlen(value2);
	
	result = ioctl(fd, HASH_IOC_PUT, &put_op);
	TEST_ASSERT(result == 0, "Second PUT operation successful");

	/* Test STATS operation */
	struct hash_stats stats;
	result = ioctl(fd, HASH_IOC_STATS, &stats);
	TEST_ASSERT(result == 0, "STATS operation successful");
	TEST_ASSERT(stats.item_count == 2, "Item count correct");
	TEST_ASSERT(stats.bucket_count > 0, "Bucket count positive");

	/* Test key update */
	char new_value1[] = "updated_value_1";
	put_op.key = key1;
	put_op.key_len = strlen(key1);
	put_op.value = new_value1;
	put_op.value_len = strlen(new_value1);
	
	result = ioctl(fd, HASH_IOC_PUT, &put_op);
	TEST_ASSERT(result == 0, "Key update successful");

	/* Verify update */
	get_op.key = key1;
	get_op.key_len = strlen(key1);
	get_op.value = retrieved_value;
	get_op.value_len = sizeof(retrieved_value);
	
	result = ioctl(fd, HASH_IOC_GET, &get_op);
	TEST_ASSERT(result == 0, "GET after update successful");
	TEST_ASSERT(memcmp(retrieved_value, new_value1, get_op.value_len) == 0, "Updated value correct");

	/* Test item count unchanged after update */
	result = ioctl(fd, HASH_IOC_STATS, &stats);
	TEST_ASSERT(result == 0 && stats.item_count == 2, "Item count unchanged after update");

	/* Test DELETE operation */
	struct hash_operation delete_op = {
		.key = key1,
		.key_len = strlen(key1)
	};
	
	result = ioctl(fd, HASH_IOC_DELETE, &delete_op);
	TEST_ASSERT(result == 0, "DELETE operation successful");

	/* Test GET after delete (should fail) */
	get_op.key = key1;
	get_op.key_len = strlen(key1);
	result = ioctl(fd, HASH_IOC_GET, &get_op);
	TEST_ASSERT(result != 0, "GET after DELETE fails as expected");

	/* Test stats after delete */
	result = ioctl(fd, HASH_IOC_STATS, &stats);
	TEST_ASSERT(result == 0 && stats.item_count == 1, "Item count decremented after delete");

	close(fd);
	TEST_SUMMARY();
}

/*
 * Test error conditions
 */
int test_error_conditions(void)
{
	TEST_START("Error Conditions");
	
	int fd = open("/dev/storage-hash", O_RDWR);
	if (fd < 0) {
		printf("ERROR: Cannot open /dev/storage-hash\n");
		return 1;
	}

	/* Test GET with non-existent key */
	char nonexistent_key[] = "nonexistent";
	char buffer[256];
	struct hash_operation get_op = {
		.key = nonexistent_key,
		.key_len = strlen(nonexistent_key),
		.value = buffer,
		.value_len = sizeof(buffer)
	};
	
	int result = ioctl(fd, HASH_IOC_GET, &get_op);
	TEST_ASSERT(result != 0, "GET with non-existent key fails");

	/* Test DELETE with non-existent key */
	struct hash_operation delete_op = {
		.key = nonexistent_key,
		.key_len = strlen(nonexistent_key)
	};
	
	result = ioctl(fd, HASH_IOC_DELETE, &delete_op);
	TEST_ASSERT(result != 0, "DELETE with non-existent key fails");

	/* Test PUT with zero-length key */
	char value[] = "test_value";
	struct hash_operation put_op = {
		.key = NULL,
		.key_len = 0,
		.value = value,
		.value_len = strlen(value)
	};
	
	result = ioctl(fd, HASH_IOC_PUT, &put_op);
	TEST_ASSERT(result != 0, "PUT with zero-length key fails");

	/* Test PUT with zero-length value */
	char key[] = "test_key";
	put_op.key = key;
	put_op.key_len = strlen(key);
	put_op.value = NULL;
	put_op.value_len = 0;
	
	result = ioctl(fd, HASH_IOC_PUT, &put_op);
	TEST_ASSERT(result != 0, "PUT with zero-length value fails");

	close(fd);
	TEST_SUMMARY();
}

/*
 * Performance test - measure operations per second
 */
int test_performance(void)
{
	TEST_START("Performance");
	
	int fd = open("/dev/storage-hash", O_RDWR);
	if (fd < 0) {
		printf("ERROR: Cannot open /dev/storage-hash\n");
		return 1;
	}

	const int num_operations = 1000;
	char key_buffer[64];
	char value_buffer[128];
	int i, successful_ops = 0;

	printf("Testing %d PUT operations...\n", num_operations);
	
	/* Time PUT operations */
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	
	for (i = 0; i < num_operations; i++) {
		snprintf(key_buffer, sizeof(key_buffer), "perf_key_%d", i);
		snprintf(value_buffer, sizeof(value_buffer), "perf_value_%d_with_some_extra_data", i);
		
		struct hash_operation put_op = {
			.key = key_buffer,
			.key_len = strlen(key_buffer),
			.value = value_buffer,
			.value_len = strlen(value_buffer)
		};
		
		if (ioctl(fd, HASH_IOC_PUT, &put_op) == 0) {
			successful_ops++;
		}
	}
	
	clock_gettime(CLOCK_MONOTONIC, &end);
	
	double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
	double ops_per_sec = successful_ops / elapsed;
	
	printf("PUT Performance: %.0f operations/second (%d/%d successful)\n", 
		ops_per_sec, successful_ops, num_operations);
	TEST_ASSERT(successful_ops == num_operations, "All PUT operations successful");

	/* Test GET performance */
	successful_ops = 0;
	char retrieved_buffer[256];
	
	clock_gettime(CLOCK_MONOTONIC, &start);
	
	for (i = 0; i < num_operations; i++) {
		snprintf(key_buffer, sizeof(key_buffer), "perf_key_%d", i);
		
		struct hash_operation get_op = {
			.key = key_buffer,
			.key_len = strlen(key_buffer),
			.value = retrieved_buffer,
			.value_len = sizeof(retrieved_buffer)
		};
		
		if (ioctl(fd, HASH_IOC_GET, &get_op) == 0) {
			successful_ops++;
		}
	}
	
	clock_gettime(CLOCK_MONOTONIC, &end);
	
	elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
	ops_per_sec = successful_ops / elapsed;
	
	printf("GET Performance: %.0f operations/second (%d/%d successful)\n", 
		ops_per_sec, successful_ops, num_operations);
	TEST_ASSERT(successful_ops == num_operations, "All GET operations successful");

	/* Check final stats */
	struct hash_stats stats;
	int result = ioctl(fd, HASH_IOC_STATS, &stats);
	TEST_ASSERT(result == 0, "Final STATS operation successful");
	printf("Final stats: %u items, %u buckets, %u bytes memory\n",
		stats.item_count, stats.bucket_count, stats.memory_usage);

	close(fd);
	TEST_SUMMARY();
}

int main(int argc, char *argv[])
{
	int total_failures = 0;
	
	printf("Hash Storage Engine Test Suite\n");
	printf("==============================\n");

	total_failures += test_basic_operations();
	total_failures += test_error_conditions();
	total_failures += test_performance();

	printf("\n=== OVERALL RESULTS ===\n");
	if (total_failures == 0) {
		printf("ALL TESTS PASSED!\n");
	} else {
		printf("SOME TESTS FAILED: %d failure(s)\n", total_failures);
	}

	return total_failures;
}