#include "luhn.h"
#include <stddef.h>
bool luhn(const char *num) {
  if (num == NULL || *num == '\0')
    return false;
  int jump = 1;
  int sum = 0;
  while (*(num + jump - 1) != '\0' && *(num + jump) != '/0') {
    int ev_nu = (*(num + jump) - '0');
    int odd_nu = (*(num + jump - 1) - '0');
    int sub_dub = ev_nu * 2;
    if (sub_dub > 9)
      sub_dub -= 9;
    sum += sub_dub + odd_nu;
    jump += 2;
  }
  if (*(num + jump - 1) != '\0')
    sum += (*(num + jump - 1) - '0');
  if(sum % 10 == 0)
    return true;
  else
    return false;
}