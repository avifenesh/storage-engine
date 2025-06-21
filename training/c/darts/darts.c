#include "darts.h"
#define ROOT_OF_FMAX 1.844674e+19f
#define HALF_FMAX 1.701412e+38f

typedef enum {
  SQUARED_OUTER_CIRCLE_RADIUS = 100,
  SQUARED_MIDDLE_CIRCLE_RADIUS = 25,
  SQUARED_INNER_CIRCLE_RADIUS = 1
} squared_radius_t;

static float square_distance_from_center(coordinate_t landing_position) {
  if (fabsf(landing_position.x) > ROOT_OF_FMAX ||
      fabsf(landing_position.y) > ROOT_OF_FMAX)
    return OVERFLOW;

  float squared_x = landing_position.x * landing_position.x;
  float squared_y = landing_position.y * landing_position.y;

  if (squared_x > HALF_FMAX || squared_y > HALF_FMAX)
    return OVERFLOW;

  return (squared_x + squared_y);
}

int score(coordinate_t landing_position) {

  float square_distance = square_distance_from_center(landing_position);

  if (square_distance == OVERFLOW)
    return TOO_FAR_ERROR;

  if (square_distance > SQUARED_OUTER_CIRCLE_RADIUS)
    return 0;
  else if (square_distance > SQUARED_MIDDLE_CIRCLE_RADIUS)
    return 1;
  else if (square_distance > SQUARED_INNER_CIRCLE_RADIUS)
    return 5;
  else
    return 10;
}