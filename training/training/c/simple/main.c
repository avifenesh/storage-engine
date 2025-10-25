#include <pthread.h>
#include <stdio.h>
#include <string.h>

void *
thread_function(void *arg)
{
	long *counter = (long *)arg;
	for (int i = 0; i < 100000000; i++) {
		*counter = *counter + 1;
	}

	printf("Thread finished. Final value: %ld\n", *counter);
	return NULL;
}

int
main(void)
{
	long value1 = 42;
	long value2 = 100;
	pthread_t thread1;
	pthread_t thread2;
	int rc;

	rc = pthread_create(&thread1, NULL, thread_function, (void *)&value1);
	if (rc != 0) {
		fprintf(stderr, "pthread_create(thread1) failed: %s\n",
			strerror(rc));
		return 1;
	}
	rc = pthread_create(&thread2, NULL, thread_function, (void *)&value2);
	if (rc != 0) {
		fprintf(stderr, "pthread_create(thread2) failed: %s\n",
			strerror(rc));
		return 1;
	}

	rc = pthread_join(thread1, NULL);
	if (rc != 0) {
		fprintf(stderr, "pthread_join(thread1) failed: %s\n",
			strerror(rc));
		return 1;
	}
	rc = pthread_join(thread2, NULL);
	if (rc != 0) {
		fprintf(stderr, "pthread_join(thread2) failed: %s\n",
			strerror(rc));
		return 1;
	}
	return 0;
}