#include "luhn.h"
#include <stddef.h>
#include <string.h>
#include <ctype.h>

static int validate_and_count(const char *num) {
  if (num == NULL || *num == '\0')
    return -1;
  int count = 0;
  while (*num != '\0') {
    if (isspace(*num)) {
      num++;
      continue;
    }
    if (*num < '0' || *num > '9')
      return -1;
    count++;
    num++;
  }
  return count;
}

bool luhn(const char *num) {
  int count = validate_and_count(num);
  if (count < 2)
    return false;
  int sum = 0;
  bool double_digit = false;
  for (int i = strlen(num) - 1; i >= 0; i--) {
    if (isspace(num[i]))
      continue;
    int digit = num[i] - '0';
    if (double_digit) {
      digit *= 2;
      if (digit > 9)
        digit -= 9;
    }
    sum += digit;
    double_digit = !double_digit;
  }
  return (sum % 10) == 0;
}

