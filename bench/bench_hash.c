#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "storage/hash_engine.h"

static inline double
ns_to_ms(int64_t ns)
{
	return (double)ns / 1e6;
}

static int64_t
now_ns(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int
main(int argc, char **argv)
{
	int n = (argc > 1) ? atoi(argv[1]) : 200000;
	struct hash_engine engine;
	char keybuf[32];
	char valbuf[32];
	int i;
	int64_t t0, t1, t2;
	volatile int sink = 0;

	if (hash_engine_init(&engine, DEFAULT_BUCKET_COUNT) != 0) {
		fprintf(stderr, "init failed\n");
		return 1;
	}

	t0 = now_ns();
	for (i = 0; i < n; ++i) {
		int k = i * 2654435761u; // mix
		int v = i;
		memcpy(keybuf, &k, sizeof(k));
		memcpy(valbuf, &v, sizeof(v));
		hash_put(&engine, keybuf, sizeof(k), valbuf, sizeof(v));
	}
	t1 = now_ns();
	for (i = 0; i < n; ++i) {
		int k = i * 2654435761u;
		const void *out = NULL;
		size_t out_len = 0;
		memcpy(keybuf, &k, sizeof(k));
		if (hash_get(&engine, keybuf, sizeof(k), &out, &out_len) == 0
		    && out && out_len == sizeof(int)) {
			int tmp;
			memcpy(&tmp, out, sizeof(tmp));
			sink ^= tmp;
		}
	}
	t2 = now_ns();

	{
		double put_ms = ns_to_ms(t1 - t0);
		double get_ms = ns_to_ms(t2 - t1);
		double put_mops = (double)n / (put_ms / 1000.0) / 1e6;
		double get_mops = (double)n / (get_ms / 1000.0) / 1e6;

		printf("hash_bench n=%d put=%.2f ms (%.3f Mops) get=%.2f ms "
		       "(%.3f Mops) sink=%d\n",
		       n, put_ms, put_mops, get_ms, get_mops, sink);
	}

	hash_engine_destroy(&engine);
	return 0;
}
