#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>

typedef struct {
  char text[6];
} clock_t;

clock_t clock_create(int hour, int minute);
clock_t clock_add(clock_t clock, int minute_add);
clock_t clock_subtract(clock_t clock, int minute_subtract);
bool clock_is_equal(clock_t a, clock_t b);

#endif
