#include "collatz_conjecture.h"

int steps(int start) {
  if (start <= 0) return -1;
  if (start == 1) return 0;

  if (start % 2 == 0)
    return steps(start / 2) + 1;
  else
    return steps(3 * start + 1) + 1;
}
