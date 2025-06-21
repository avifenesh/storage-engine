#ifndef DARTS_H
#define DARTS_H
#include <float.h>
#include <math.h>

typedef struct {
   float x;
   float y;
} coordinate_t;

int score(coordinate_t landing_position);

#define TOO_FAR_ERROR -1
#define OVERFLOW -1.0

#endif
