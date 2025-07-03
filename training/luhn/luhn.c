#include <stddef.h>
#include "luhn.h"

typedef struct {
  bool is_seconed;
  int sum;
  bool above_two_digits;
} tracking_data;

static void luhn_recursive_run(const char *num, tracking_data *data) {
  if (num == NULL){
    return;
  }

  char cha = *num;
  if (cha == '\0')
    return;

  if (cha == ' '){
    luhn_recursive_run(num + 1, data);
    return;
  }

  if(cha < '0' || cha > '9'){
    data->sum = -1;
    return;
  }
  
  luhn_recursive_run(num + 1, data);
  if (data->sum < 0)
    return;

  int digit = cha - '0';

  if(data->is_seconed) {
    data->above_two_digits = true;
    digit *= 2;
    if (digit > 9)
      digit -= 9;
  }
  data->sum += digit;
  data->is_seconed = !data->is_seconed;
  return;
}

bool luhn(const char *num) {
  tracking_data data = {false, 0, false};
  tracking_data *ptr = &data;
  luhn_recursive_run(num, ptr);
  return data.sum >= 0 && data.sum % 10 == 0 && data.above_two_digits;
}
