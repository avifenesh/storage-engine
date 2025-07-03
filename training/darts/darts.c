#include "darts.h"
#define CIRCLE_LIMIT 10.0f
#define OUT_OF_CIRCLE -1.0f
typedef enum {
  SQUARED_OUTER_CIRCLE_RADIUS = 100,
  SQUARED_MIDDLE_CIRCLE_RADIUS = 25,
  SQUARED_INNER_CIRCLE_RADIUS = 1
} squared_radius_t;

static float square_distance_from_center(coordinate_t landing_position) {
  if (landing_position.x > CIRCLE_LIMIT || landing_position.y > CIRCLE_LIMIT) return OUT_OF_CIRCLE;

  float squared_x = landing_position.x * landing_position.x;
  float squared_y = landing_position.y * landing_position.y;

  return (squared_x + squared_y);
}

int score(coordinate_t landing_position) {

  float square_distance = square_distance_from_center(landing_position);

  if (square_distance > SQUARED_OUTER_CIRCLE_RADIUS || square_distance == OUT_OF_CIRCLE)
    return 0;
  else if (square_distance > SQUARED_MIDDLE_CIRCLE_RADIUS)
    return 1;
  else if (square_distance > SQUARED_INNER_CIRCLE_RADIUS)
    return 5;
  else
    return 10;
}
