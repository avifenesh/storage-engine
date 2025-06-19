#include "difference_of_squares.h"
// For the exercise 2 will be an ERROR code
unsigned int sum_of_squares(unsigned int number) {
  if (number >= 1291) // if number is larger than 1291, the result will overflow
    return OVERFLOW;

  if (number == 0 || number == 1)
    return number;

  return (number * (number + 1) * (2 * number + 1)) / 6;
}

unsigned int square_of_sum(unsigned int number) {
  if (number >=
      65536) // if number is larger than 65536, the result will overflow
    return OVERFLOW;

  if (number == 0 || number == 1)
    return number;
  return (number * number * (number + 1) * (number + 1)) / 4;
}

unsigned int difference_of_squares(unsigned int number) {
  // Safety - the functions above will return OVERFLOW if the number is too
  // large, and are responsible for checking that.
  return (square_of_sum(number) - sum_of_squares(number));
}