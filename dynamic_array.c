#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *head;
    int size;
} dyn_array;

int append(dyn_array *arr, int value)
{
    int *new_head = (int *)realloc((arr->head), (arr->size + 1)*sizeof(int));
    if(new_head == NULL) return -1;
    arr->head = new_head;
    *(arr->head + arr->size) = value; 
    arr->size++;
    return 0;
}

int main(void)
{
    dyn_array arr;
    dyn_array *p_arr = &arr;
    arr.size = 0;
    arr.head = NULL;



    append(p_arr, 7);
    append(p_arr, 6);

    printf("The array should contain two element now - %d and %d and its size is %d\n", *(arr.head), *(arr.head + 1), arr.size);

    free(arr.head);

    arr.head = NULL;
    return 0;
}

// Add get function, create and free;
// For real performance, use size and capacity