#include <stdio.h>

int
main(void)
{
	int i = 10;

	printf("The value of i is %d\n", i);
	printf("The address of i is %p\n", (void *)&i);
}