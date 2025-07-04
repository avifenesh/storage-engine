#include <stdio.h>

void increament(int *p) { *p = *p + 1; }

int main(void)
{
	int i = 10;
	int *j = &i;
	printf("i is %d\n", i);
	printf("i is also %d\n", *j);

	increament(j);

	printf("i is %d\n", i);
}
