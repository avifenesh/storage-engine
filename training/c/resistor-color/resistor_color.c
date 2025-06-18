#include "resistor_color.h"

unsigned char color_code(resistor_band_t color) { return (char)color; }

const resistor_band_t *colors(void) {
#define X(name, value) name,
  static resistor_band_t color_array[] = {RESISTOR_COLORS};
#undef X

  return color_array;
}