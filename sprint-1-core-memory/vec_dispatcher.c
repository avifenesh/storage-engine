#include <arm_neon.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int (*operation_func_t)(const void *a, const void *b, void *result);

typedef struct {
  const char *name;
  operation_func_t func;
} operation_dispatch_t;

typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_DOUBLE } data_type_t;

// operations:
// add
// multiplie
// dot product

void add(void *a, void *b, void *result) { *((float16_t *)) }