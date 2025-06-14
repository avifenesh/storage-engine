#include <stdio.h>
#include <stdint.h>


typedef struct {
    uint16_t *x;
    uint16_t *y;
    uint16_t *z;
    uint16_t *w;
} aligned_vec_t __attribute__((aligned(128)));