#include "eliuds_eggs.h"
int egg_count(int decimal_rep) {
  int eggs = 0;
  if (decimal_rep < 1)
    return 0;
  while (decimal_rep > 0) {
    eggs += decimal_rep % 2 == 0 ? 0 : 1;
    decimal_rep >>= 1;
  }
  return eggs;
}