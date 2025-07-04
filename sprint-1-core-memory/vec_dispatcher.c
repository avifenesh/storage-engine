#include "vec.h"

int add_f32(const float32_t *a, const float32_t *b, float32_t *result)
{
	*result = *a + *b;
	return 0;
}

int multiply_f32(const float32_t *a, const float32_t *b, float32_t *result)
{
	*result = *a * *b;
	return 0;
}

int add_vec4_f32(const aligned_vec_t *a, const aligned_vec_t *b,
		 aligned_vec_t *result)
{
	float32x4_t neon_a = vld1q_f32(&a->x);
	float32x4_t neon_b = vld1q_f32(&b->x);
	float32x4_t neon_sum = vaddq_f32(neon_a, neon_b);
	vst1q_f32(&result->x, neon_sum);
	return 0;
}

int multiply_vec4_f32(const aligned_vec_t *a, const aligned_vec_t *b,
		      aligned_vec_t *result)
{
	float32x4_t neon_a = vld1q_f32(&a->x);
	float32x4_t neon_b = vld1q_f32(&b->x);
	float32x4_t neon_product = vmulq_f32(neon_a, neon_b);
	vst1q_f32(&result->x, neon_product);
	return 0;
}

int dot_vec4_f32(const aligned_vec_t *a, const aligned_vec_t *b,
		 float32_t *result)
{
	float32x4_t neon_a = vld1q_f32(&a->x);
	float32x4_t neon_b = vld1q_f32(&b->x);
	float32x4_t neon_product = vmulq_f32(neon_a, neon_b);

	*result = vaddvq_f32(neon_product);
	return 0;
}

typedef int (*operation_f32_t)(const float32_t *a, const float32_t *b,
			       float32_t *result);
typedef int (*operation_vec4_f32_t)(const aligned_vec_t *a,
				    const aligned_vec_t *b,
				    aligned_vec_t *result);

static const operation_f32_t f32_operations[] = {
    [ADD] = add_f32,
    [MULTIPLY] = multiply_f32,
};

static const operation_vec4_f32_t vec4_f32_operations[] = {
    [ADD] = add_vec4_f32,
    [MULTIPLY] = multiply_vec4_f32,
};

int dispatch(const data_type_t type, const op_name_t name, const void *a,
	     const void *b, void *result)
{
	if (type == TYPE_F32) {
		if (name == DOT) {
			return -1;
		}
		return f32_operations[name]((const float32_t *)a,
					    (const float32_t *)b,
					    (float32_t *)result);
	} else if (type == TYPE_VEC4_F32) {
		if (name == DOT) {
			return dot_vec4_f32((const aligned_vec_t *)a,
					    (const aligned_vec_t *)b,
					    (float32_t *)result);
		}
		return vec4_f32_operations[name]((const aligned_vec_t *)a,
						 (const aligned_vec_t *)b,
						 (aligned_vec_t *)result);
	}

	return -1;
}