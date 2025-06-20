#ifndef RESISTOR_COLOR_DUO_H
#define RESISTOR_COLOR_DUO_H

#include <stdint.h>
#include <stddef.h>
typedef enum {
  BLACK,
  BROWN,
  RED,
  ORANGE,
  YELLOW,
  GREEN,
  BLUE,
  VIOLET,
  GREY,
  WHITE
} resistor_band_t;

typedef enum { NULL_ARRAY = 404, INVALID_VAL = 405 } error_code_t;

uint16_t color_code(resistor_band_t colors[]);

#endif
