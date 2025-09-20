#include <stdio.h>
#include <time.h>

static double
now_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int
main(void)
{
	// Placeholder: add insert/search/range microbenchmarks
	double t0 = now_ms();
	// ... run tests ...
	double t1 = now_ms();
	printf("btree benchmarks placeholder: %.3f ms\n", t1 - t0);
	return 0;
}
