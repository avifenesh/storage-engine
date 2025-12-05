/**
 * @file hash_fuzz_libfuzzer.c
 * @brief LibFuzzer integration for hash table fuzzing
 *
 * Compile with:
 *   clang -fsanitize=fuzzer,address -g -O1 -I../../include \
 *         hash_fuzz_libfuzzer.c ../../src/storage/hash/bucket.c \
 *         ../../src/storage/hash/siphash.c ../../src/storage/hash/hash_engine.c
 * \ -o hash_fuzz_libfuzzer
 *
 * Run with:
 *   ./hash_fuzz_libfuzzer -max_len=4096 -timeout=10
 *
 * Generate corpus:
 *   mkdir -p corpus
 *   ./hash_fuzz_libfuzzer corpus/ -max_total_time=60
 */

#include "storage/hash_engine.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Operation types for fuzzing */
enum fuzz_operation {
	OP_PUT = 0,
	OP_GET = 1,
	OP_DELETE = 2,
	OP_GET_STATS = 3,
};

/* Fuzzing state */
static struct hash_engine global_engine;
static int engine_initialized = 0;

/* Parse fuzzer input into operations */
static void
parse_and_execute_operations(const uint8_t *data, size_t size)
{
	size_t offset;
	enum fuzz_operation op;
	uint8_t key_len;
	uint8_t value_len;
	const uint8_t *key_data;
	const uint8_t *value_data;
	const void *retrieved_value;
	size_t retrieved_len;
	uint32_t item_count;
	uint32_t bucket_count;
	uint32_t memory_usage;

	offset = 0;

	while (offset < size) {
		/* Need at least 3 bytes: operation, key_len, value_len */
		if (offset + 3 > size)
			break;

		/* Parse operation */
		op = (enum fuzz_operation)(data[offset] % 4);
		offset++;

		/* Parse key length */
		key_len = data[offset];
		offset++;

		/* Limit key length to avoid timeouts */
		if (key_len > 128)
			key_len = 128;

		if (key_len == 0)
			key_len = 1; /* Ensure non-zero key */

		/* Check if we have enough data for key */
		if (offset + key_len > size)
			break;

		key_data = &data[offset];
		offset += key_len;

		switch (op) {
		case OP_PUT:
			/* Parse value length */
			if (offset >= size)
				break;
			value_len = data[offset];
			offset++;

			/* Limit value length */
			if (value_len > 128)
				value_len = 128;

			if (value_len == 0)
				value_len = 1;

			/* Check if we have enough data for value */
			if (offset + value_len > size)
				break;

			value_data = &data[offset];
			offset += value_len;

			/* Execute PUT */
			(void)hash_put(&global_engine, key_data, key_len,
				       value_data, value_len);
			break;

		case OP_GET:
			/* Execute GET */
			(void)hash_get(&global_engine, key_data, key_len,
				       &retrieved_value, &retrieved_len);
			break;

		case OP_DELETE:
			/* Execute DELETE */
			(void)hash_delete(&global_engine, key_data, key_len);
			break;

		case OP_GET_STATS:
			/* Execute GET_STATS */
			(void)hash_engine_get_stats(&global_engine, &item_count,
						    &bucket_count,
						    &memory_usage);
			break;
		}
	}
}

/* LibFuzzer entry point */
int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	static int iteration_count = 0;

	/* Initialize engine on first run */
	if (!engine_initialized) {
		if (hash_engine_init(&global_engine, 64) != 0) {
			return 0;
		}
		engine_initialized = 1;
	}

	/* Skip empty input */
	if (size == 0)
		return 0;

	/* Parse and execute fuzzer-generated operations */
	parse_and_execute_operations(data, size);

	/* Periodically reset to avoid memory accumulation */
	/* This is a simplification - in production fuzzing, you might
	 * want more sophisticated state management */
	iteration_count++;
	if (iteration_count % 100 == 0) {
		hash_engine_destroy(&global_engine);
		if (hash_engine_init(&global_engine, 64) != 0) {
			engine_initialized = 0;
			return 0;
		}
	}

	return 0;
}
