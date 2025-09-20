#include <stdio.h>
#include <stdlib.h>

typedef struct {
	const char *name;
	int (*fn)(void);
} test_case_t;

static int
test_placeholder(void)
{
	return 0;
}

static void
run_tests(test_case_t *cases, size_t n)
{
	size_t pass = 0, fail = 0;
	for (size_t i = 0; i < n; i++) {
		int rc = cases[i].fn();
		if (rc == 0) {
			printf("PASS %s\n", cases[i].name);
			pass++;
		} else {
			printf("FAIL %s rc=%d\n", cases[i].name, rc);
			fail++;
		}
	}
	printf("Summary: %zu passed, %zu failed\n", pass, fail);
	if (fail)
		exit(1);
}

int
main(void)
{
	test_case_t cases[] = {
		{ "placeholder", test_placeholder },
	};
	run_tests(cases, sizeof(cases) / sizeof(cases[0]));
	return 0;
}
