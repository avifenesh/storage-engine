#include "grains.h"

uint64_t square(uint8_t index) {
  return (index > 64 || index == 0) ? 0 : 1UL << (index - 1);
}
uint64_t total(void) { return UINT64_MAX; }