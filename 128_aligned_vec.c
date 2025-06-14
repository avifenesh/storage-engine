#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint16_t *x;
    uint16_t *y;
    uint16_t *z;
    uint16_t *w;
} aligned_vec_t __attribute__((aligned(128)));

int main() {
  aligned_vec_t vec;

  // Allocate aligned memory for each component
  vec.x = (uint16_t *)aligned_alloc(128, sizeof(uint16_t) * 4);
  vec.y = (uint16_t *)aligned_alloc(128, sizeof(uint16_t) * 4);
  vec.z = (uint16_t *)aligned_alloc(128, sizeof(uint16_t) * 4);
  vec.w = (uint16_t *)aligned_alloc(128, sizeof(uint16_t) * 4);

  if (!vec.x || !vec.y || !vec.z || !vec.w) {
    printf("Memory allocation failed\n");
    return 1;
  }

  // Initialize values
  for (int i = 0; i < 4; i++) {
    vec.x[i] = i + 1;
    vec.y[i] = i + 2;
    vec.z[i] = i + 3;
    vec.w[i] = i + 4;
  }

  // Print alignment info
  printf("Struct alignment: %lu bytes\n", sizeof(aligned_vec_t));
  printf("Struct address: %p (alignment: %lu)\n", &vec, (uintptr_t)&vec % 128);

  // Print values
  printf("Vector components:\n");
  for (int i = 0; i < 4; i++) {
    printf("  [%d]: x=%u, y=%u, z=%u, w=%u\n", i, vec.x[i], vec.y[i], vec.z[i],
           vec.w[i]);
  }

  // Clean up
  free(vec.x);
  free(vec.y);
  free(vec.z);
  free(vec.w);

  return 0;
}