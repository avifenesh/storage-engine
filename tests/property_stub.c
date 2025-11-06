#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "storage/hash_engine.h"

#define TEST_BUCKET_COUNT 256
#define KEY_SPACE 4096
#define OPS_PER_TRIAL 1024

struct oracle_slot {
	int key;
	int value;
	size_t value_len;
	bool present;
};

static unsigned
rnd(void)
{
	return (unsigned)rand();
}

static void
run_trial(void)
{
	struct hash_engine engine;
	struct oracle_slot slots[KEY_SPACE];
	uint32_t item_count = 0;
	int expected = 0;
	int op;
	int i;

	assert(hash_engine_init(&engine, TEST_BUCKET_COUNT) == 0);
	memset(slots, 0, sizeof(slots));

	for (op = 0; op < OPS_PER_TRIAL; ++op) {
		int key_idx = (int)(rnd() % KEY_SPACE);
		struct oracle_slot *slot = &slots[key_idx];
		size_t key_len = sizeof(slot->key);
		int action = (int)(rnd() % 3);

		slot->key = key_idx;

		switch (action) {
		case 0: // insert/update
			slot->value = (int)rnd();
			slot->value_len = sizeof(slot->value);
			assert(hash_put(&engine, &slot->key, key_len,
					&slot->value, slot->value_len)
			       == 0);
			slot->present = true;
			break;
		case 1: { // get
			const void *value = NULL;
			size_t value_len = 0;
			int rc = hash_get(&engine, &slot->key, key_len, &value,
					  &value_len);
			if (slot->present) {
				assert(rc == 0);
				assert(value != NULL);
				assert(value_len == slot->value_len);
				assert(
				    memcmp(value, &slot->value, slot->value_len)
				    == 0);
			} else {
				assert(rc == -ENOENT);
			}
			break;
		}
		default: { // delete
			int rc = hash_delete(&engine, &slot->key, key_len);
			if (slot->present) {
				assert(rc == 0);
				slot->present = false;
			} else {
				assert(rc == -ENOENT);
			}
			break;
		}
		}
	}

	for (i = 0; i < KEY_SPACE; ++i) {
		if (slots[i].present) {
			const void *value = NULL;
			size_t value_len = 0;
			assert(hash_get(&engine, &slots[i].key,
					sizeof(slots[i].key), &value,
					&value_len)
			       == 0);
			assert(value_len == slots[i].value_len);
			assert(memcmp(value, &slots[i].value,
				      slots[i].value_len)
			       == 0);
			expected++;
		}
	}

	assert(hash_engine_get_stats(&engine, &item_count, NULL, NULL) == 0);
	assert(item_count == (uint32_t)expected);
	assert(hash_engine_destroy(&engine) == 0);
}

int
main(int argc, char **argv)
{
	int trials = (argc > 1) ? atoi(argv[1]) : 1000;
	int t;

	srand((unsigned)time(NULL));
	for (t = 0; t < trials; ++t) {
		run_trial();
	}

	return 0;
}

