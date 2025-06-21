#include "binary.h"
#include <stddef.h>

int convert(const char *input){
    if(input == NULL)
        return INVALID;
    
    int sum = 0;
    
    while (*input != '\0'){
        if (*input == '0') {
            sum = sum * 2 + 0;
        }
        else if (*input == '1') {
            sum = sum * 2 + 1;
        }
        else {
            return INVALID;
        }
        input++;
    }
    
    return sum;
}