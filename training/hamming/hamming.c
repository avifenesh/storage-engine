#include "hamming.h"

int compute(const char *lhs, const char *rhs) {
  int distance = 0;
  while (*lhs && *rhs) {

    if (*rhs != *lhs)
      distance++;

    rhs++;
    lhs++;
  }

  if (*rhs || *lhs)
    return ERROR;
  return distance;
}
