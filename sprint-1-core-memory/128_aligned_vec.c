#include <arm_neon.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  float x;
  float y;
  float z;
  float w;
} aligned_vec_t __attribute__((aligned(16)));
void print_vec(const char *name, const aligned_vec_t *vec) {
  printf("%s: x=%.2f, y=%.2f, z=%.2f, w=%.2f (addr: %p)\n", name, vec->x,
         vec->y, vec->z, vec->w, (void *)vec);
}

int main(void) {
  aligned_vec_t vec_a;
  aligned_vec_t vec_b;
  aligned_vec_t vec_result_neon;
  aligned_vec_t vec_result_scalar;

  vec_a.x = 1.0f;
  vec_a.y = 2.0f;
  vec_a.z = 3.0f;
  vec_a.w = 4.0f;
  vec_b.x = 5.0f;
  vec_b.y = 6.0f;
  vec_b.z = 7.0f;
  vec_b.w = 8.0f;

  printf("Input Vectors:\n");
  print_vec("vec_a", &vec_a);
  print_vec("vec_b", &vec_b);
  printf("sizeof(aligned_vec_t): %zu bytes\n\n", sizeof(aligned_vec_t));

  printf("Performing NEON SIMD addition (vec_a + vec_b):\n");

  float32x4_t neon_a = vld1q_f32(&vec_a.x);
  float32x4_t neon_b = vld1q_f32(&vec_b.x);

  float32x4_t neon_sum = vaddq_f32(neon_a, neon_b);

  vst1q_f32(&vec_result_neon.x, neon_sum);

  print_vec("vec_result_neon", &vec_result_neon);
  printf("\n");

  printf("Performing scalar addition for comparison:\n");
  vec_result_scalar.x = vec_a.x + vec_b.x;
  vec_result_scalar.y = vec_a.y + vec_b.y;
  vec_result_scalar.z = vec_a.z + vec_b.z;
  vec_result_scalar.w = vec_a.w + vec_b.w;
  print_vec("vec_result_scalar", &vec_result_scalar);

  if (vec_result_neon.x == vec_result_scalar.x &&
      vec_result_neon.y == vec_result_scalar.y &&
      vec_result_neon.z == vec_result_scalar.z &&
      vec_result_neon.w == vec_result_scalar.w) {
    printf(
        "\nNEON and scalar results match! SIMD likely worked as expected.\n");
  } else {
    printf("\nNEON and scalar results DO NOT match! Check your logic or "
           "compilation.\n");
  }

  return EXIT_SUCCESS;
}