#include "binary.h"
#include <stddef.h>

int convert(const char *input){
    if(input == NULL)
        return INVALID;

    // While probably not expected in this exercise, the input size is not
    // limited, and since its in string we have no way to know what int size
    // will be sufficient. The decision to go with ULLI is for best effort to
    // adjust for user input. Overflow is still an option.
    unsigned long long int sum = 0;

    while (*input != '\0'){
      sum <<= 1;
      if (*input == '1') {
        sum++;
      } else if (*input != '0') {
        return INVALID;
      }
        input++;
    }
    
    return sum;
}