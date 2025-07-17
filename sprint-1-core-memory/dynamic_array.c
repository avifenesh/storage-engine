#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int *data;
	size_t size;
	size_t capacity;
} dyn_array_t;

int
append(dyn_array_t *arr, int value)
{
	if (!(arr->capacity > arr->size)) {
		arr->capacity = arr->capacity ? arr->capacity * 2 : 2;
		arr->data = realloc(arr->data, arr->capacity * sizeof(int));
		if (!arr->data) {
			fprintf(stderr, "Memory allocation failed\n");
			return -1;
		}
	} else
		arr->capacity = arr->size + 1;
	*(arr->data + arr->size) = value;
	arr->size++;
	return 0;
}

void
array_init(dyn_array_t *arr)
{
	arr->data = NULL;
	arr->size = 0;
	arr->capacity = 0;
}

void
free_array(dyn_array_t *arr)
{
	if (arr->data) {
		free(arr->data);
		arr->data = NULL;
	}
	arr->size = 0;
	arr->capacity = 0;
}

int
main(void)
{
	dyn_array_t arr;
	dyn_array_t *arr_ptr = &arr;
	array_init(arr_ptr);
	for (int i = 0; i < 10; i++) {
		if (append(arr_ptr, i) != 0) {
			fprintf(stderr, "Failed to append value %d\n", i);
			free_array(arr_ptr);
			return EXIT_FAILURE;
		}
	}
	printf("Dynamic array contents:\n");
	for (size_t i = 0; i < arr.size; i++) {
		printf("%d ", arr.data[i]);
	}
	printf("\n");
	free_array(arr_ptr);
	return EXIT_SUCCESS;
}