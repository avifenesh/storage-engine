#include <stddef.h>
#include "luhn.h"

typedef struct {
  bool is_seconed;
  int sum;
  bool above_two_digits;
} recursive_data;

static void recursive_response(const char *num, recursive_data *data) {
  if (num == NULL){
    return;
  }

  char cha = *num;
  if (cha == '\0')
    return;

  if (cha == ' '){
     recursive_response(num + 1, data);
    return;
  }

  if(cha < '0' || cha > '9'){
    data->sum = -1;
    return;
  }
  
  recursive_response(num + 1, data);
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
  recursive_data data = {false, 0, false};
  recursive_data *ptr = &data;
  recursive_response(num, ptr);
  return data.sum >= 0 && data.sum % 10 == 0 && data.above_two_digits;
}
