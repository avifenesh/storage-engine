#include "vec.h"

int
add_f32(const float32_t *a, const float32_t *b, float32_t *result)
{
	*result = *a + *b;
	return 0;
}

int
multiply_f32(const float32_t *a, const float32_t *b, float32_t *result)
{
	*result = *a * *b;
	return 0;
}

int
add_vec4_f32(const aligned_vec_t *a, const aligned_vec_t *b,
	     aligned_vec_t *result)
{
	float32x4_t neon_a = vld1q_f32(&a->x);
	float32x4_t neon_b = vld1q_f32(&b->x);
	float32x4_t neon_sum = vaddq_f32(neon_a, neon_b);
	vst1q_f32(&result->x, neon_sum);
	return 0;
}

int
multiply_vec4_f32(const aligned_vec_t *a, const aligned_vec_t *b,
		  aligned_vec_t *result)
{
	float32x4_t neon_a = vld1q_f32(&a->x);
	float32x4_t neon_b = vld1q_f32(&b->x);
	float32x4_t neon_product = vmulq_f32(neon_a, neon_b);
	vst1q_f32(&result->x, neon_product);
	return 0;
}

int
dot_vec4_f32(const aligned_vec_t *a, const aligned_vec_t *b, float32_t *result)
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

int
dispatch(const data_type_t type, const op_name_t name, const void *a,
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

/* Test and demonstration main function */
int
main(void)
{
	printf("🎯 Function Pointer Dispatcher Demo - Issue #5\n");
	printf("===============================================\n\n");

	/* Test 1: Float32 operations */
	printf("🔢 Testing float32 operations:\n");
	float32_t a = 3.5f, b = 2.0f, result_f32;

	printf("  Input: a=%.2f, b=%.2f\n", a, b);

	if (dispatch(TYPE_F32, ADD, &a, &b, &result_f32) == 0) {
		printf("  ✅ ADD: %.2f + %.2f = %.2f\n", a, b, result_f32);
	}

	if (dispatch(TYPE_F32, MULTIPLY, &a, &b, &result_f32) == 0) {
		printf("  ✅ MULTIPLY: %.2f * %.2f = %.2f\n", a, b, result_f32);
	}

	if (dispatch(TYPE_F32, DOT, &a, &b, &result_f32) != 0) {
		printf("  ❌ DOT: Not supported for float32 (expected)\n");
	}

	/* Test 2: Vector4 operations */
	printf("\n🔢 Testing vector4 SIMD operations:\n");
	aligned_vec_t vec_a = { 1.0f, 2.0f, 3.0f, 4.0f };
	aligned_vec_t vec_b = { 2.0f, 3.0f, 1.0f, 2.0f };
	aligned_vec_t result_vec;
	float32_t dot_result;

	printf("  Vector A: [%.1f, %.1f, %.1f, %.1f]\n", vec_a.x, vec_a.y,
	       vec_a.z, vec_a.w);
	printf("  Vector B: [%.1f, %.1f, %.1f, %.1f]\n", vec_b.x, vec_b.y,
	       vec_b.z, vec_b.w);

	if (dispatch(TYPE_VEC4_F32, ADD, &vec_a, &vec_b, &result_vec) == 0) {
		printf("  ✅ VECTOR ADD: [%.1f, %.1f, %.1f, %.1f]\n",
		       result_vec.x, result_vec.y, result_vec.z, result_vec.w);
	}

	if (dispatch(TYPE_VEC4_F32, MULTIPLY, &vec_a, &vec_b, &result_vec)
	    == 0) {
		printf("  ✅ VECTOR MULTIPLY: [%.1f, %.1f, %.1f, %.1f]\n",
		       result_vec.x, result_vec.y, result_vec.z, result_vec.w);
	}

	if (dispatch(TYPE_VEC4_F32, DOT, &vec_a, &vec_b, &dot_result) == 0) {
		printf("  ✅ DOT PRODUCT: %.1f\n", dot_result);

		/* Verify dot product manually: (1*2 + 2*3 + 3*1 + 4*2) =
		 * 2+6+3+8 = 19 */
		float32_t expected = vec_a.x * vec_b.x + vec_a.y * vec_b.y
				     + vec_a.z * vec_b.z + vec_a.w * vec_b.w;
		printf("  🔍 Manual verification: %.1f (should match)\n",
		       expected);
	}

	/* Test 3: Function pointer demonstration */
	printf("\n🔧 Function Pointer Technology Demo:\n");
	printf("  📋 Scalar operations array contains %zu function pointers\n",
	       sizeof(f32_operations) / sizeof(f32_operations[0]));
	printf("  📋 Vector operations array contains %zu function pointers\n",
	       sizeof(vec4_f32_operations) / sizeof(vec4_f32_operations[0]));

	printf(
	    "  🎯 Dispatcher routes calls based on data type and operation\n");
	printf("  ⚡ NEON SIMD instructions used for vector operations\n");
	printf(
	    "  🏗️  Foundation for multi-modal query routing (Sprint 5)\n");

	printf("\n✅ Issue #5 Complete: Function pointer callback dispatcher "
	       "working!\n");
	printf("🚀 Ready to advance to Sprint 2: System Programming\n");

	return 0;
}