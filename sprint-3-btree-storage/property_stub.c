#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Property-test scaffold for your B+ tree.
// TODO: add randomized inserts/deletes/searches and verify invariants.

int
main(int argc, char **argv)
{
	int ops = (argc > 1) ? atoi(argv[1]) : 5000;
	srand((unsigned)time(NULL));
	printf("[btree property stub] ops=%d\n", ops);
	// TODO: after you implement, verify sorted order, min/max fill, height
	// consistency.
	printf("[btree property stub] TODO: implement checks and enable "
	       "asserts\n");
	return 0;
}
