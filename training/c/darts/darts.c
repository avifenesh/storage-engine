#include "darts.h"

float square_distance_from_center(coordinate_t landing_position) {
  return ((landing_position.x * landing_position.x) +
               (landing_position.y * landing_position.y));
}

int score(coordinate_t landing_position) {
  if (landing_position.x > __FLT_MAX__ || landing_position.x < -__FLT_MAX__ ||
      landing_position.y > __FLT_MAX__ || landing_position.y < -__FLT_MAX__)
    return TOO_FAR_ERROR;
    
  float square_distance = square_distance_from_center(landing_position);

  if (square_distance > SQUARED_OUTER_CIRCLE_RADIUS)
    return 0;
  else if (square_distance > SQUARED_MIDDLE_CIRCLE_RADIUS)
    return 1;
  else if (square_distance > SQUARED_INNER_CIRCLE_RADIUS)
    return 5;
  else
    return 10;
}