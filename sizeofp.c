#include <stdio.h>
#include <sys/types.h>
int main(void)
{
	int *p;

	printf("%zu\n", sizeof(int));
	printf("%zu\n", sizeof(p));
	printf("%zu\n", sizeof(*p));
	printf("%zu\n", sizeof(u_long));
}
