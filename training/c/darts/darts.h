#ifndef DARTS_H
#define DARTS_H

typedef struct {
   float x;
   float y;
} coordinate_t;

int score(coordinate_t landing_position);
float square_distance_from_center(coordinate_t landing_position);

typedef enum {
   SQUARED_OUTER_CIRCLE_RADIUS = 100,
   SQUARED_MIDDLE_CIRCLE_RADIUS = 25,
   SQUARED_INNER_CIRCLE_RADIUS = 1
} squared_radius_t;

#define TOO_FAR_ERROR -1

#endif
