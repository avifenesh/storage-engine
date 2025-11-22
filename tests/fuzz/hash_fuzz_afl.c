/**
 * @file hash_fuzz_afl.c
 * @brief AFL++ integration for hash table fuzzing
 *
 * Compile with AFL++:
 *   afl-clang-fast -fsanitize=address -g -O1 -I../../include \
 *                  hash_fuzz_afl.c ../../src/storage/hash/*.c \
 *                  -o hash_fuzz_afl
 *
 * Or with persistent mode (faster):
 *   afl-clang-fast -fsanitize=address -g -O1 -I../../include \
 *                  -DAFL_PERSISTENT_MODE \
 *                  hash_fuzz_afl.c ../../src/storage/hash/*.c \
 *                  -o hash_fuzz_afl
 *
 * Run with:
 *   afl-fuzz -i input_corpus/ -o output/ -- ./hash_fuzz_afl
 *
 * Create initial corpus:
 *   mkdir -p input_corpus
 *   echo "PUT test_key test_value" > input_corpus/test1.txt
 *   echo "GET test_key" > input_corpus/test2.txt
 *   echo "DELETE test_key" > input_corpus/test3.txt
 */

#include "storage/hash_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 4096

/* Operation parsing and execution */
static void
execute_text_operations(const char *input, size_t size)
{
	struct hash_engine engine;
	char *line;
	char *saveptr;
	char *input_copy;
	char *token;
	char *operation;
	char *key;
	char *value;
	const void *retrieved_value;
	size_t retrieved_len;

	/* Initialize engine */
	if (hash_engine_init(&engine, 64) != 0) {
		return;
	}

	/* Make a mutable copy of input */
	input_copy = malloc(size + 1);
	if (!input_copy) {
		hash_engine_destroy(&engine);
		return;
	}
	memcpy(input_copy, input, size);
	input_copy[size] = '\0';

	/* Parse line by line */
	line = strtok_r(input_copy, "\n", &saveptr);
	while (line != NULL) {
		/* Parse operation */
		operation = strtok(line, " ");
		if (!operation) {
			line = strtok_r(NULL, "\n", &saveptr);
			continue;
		}

		if (strcmp(operation, "PUT") == 0) {
			key = strtok(NULL, " ");
			value = strtok(NULL, " ");
			if (key && value) {
				(void)hash_put(&engine, key, strlen(key),
					       value, strlen(value));
			}
		} else if (strcmp(operation, "GET") == 0) {
			key = strtok(NULL, " ");
			if (key) {
				(void)hash_get(&engine, key, strlen(key),
					       &retrieved_value,
					       &retrieved_len);
			}
		} else if (strcmp(operation, "DELETE") == 0) {
			key = strtok(NULL, " ");
			if (key) {
				(void)hash_delete(&engine, key, strlen(key));
			}
		} else if (strcmp(operation, "STATS") == 0) {
			uint32_t item_count;
			uint32_t bucket_count;
			uint32_t memory_usage;
			(void)hash_engine_get_stats(&engine, &item_count,
						    &bucket_count,
						    &memory_usage);
		}

		line = strtok_r(NULL, "\n", &saveptr);
	}

	free(input_copy);
	hash_engine_destroy(&engine);
}

int
main(int argc, char **argv)
{
	char input[MAX_INPUT_SIZE];
	ssize_t bytes_read;

#ifdef AFL_PERSISTENT_MODE
	/* AFL++ persistent mode for performance */
	while (__AFL_LOOP(1000)) {
		bytes_read = read(STDIN_FILENO, input, sizeof(input) - 1);
		if (bytes_read <= 0)
			continue;

		input[bytes_read] = '\0';
		execute_text_operations(input, (size_t)bytes_read);
	}
#else
	/* Standard mode */
	bytes_read = read(STDIN_FILENO, input, sizeof(input) - 1);
	if (bytes_read > 0) {
		input[bytes_read] = '\0';
		execute_text_operations(input, (size_t)bytes_read);
	}
#endif

	(void)argc;
	(void)argv;
	return 0;
}
