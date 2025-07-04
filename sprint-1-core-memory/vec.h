#ifndef VEC_H
#define VEC_H
#include <arm_neon.h>
#include <stdint.h>
typedef struct {
	float32_t x;
	float32_t y;
	float32_t z;
	float32_t w;
} aligned_vec_t __attribute__((aligned(16)));

typedef enum { TYPE_F32, TYPE_VEC4_F32 } data_type_t;

typedef enum { ADD, MULTIPLY, DOT } op_name_t;

int dispatch(const data_type_t type, const op_name_t name, const void *a,
	     const void *b, void *result);

#endif
