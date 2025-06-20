#include "resistor_color_duo.h"

uint16_t color_code(resistor_band_t colors[]){
  if (colors == NULL)
    return NULL_ARRAY;

  // This check is a best effort check, the Array might
  // contain garbage data which is inbound somehow. A lot
  // of bad luck, but the tests force me to use the same
  // API so i cant make it more secure.
  if (colors[0] < BLACK || colors[0] > WHITE || colors[1] < BLACK ||
      colors[1] > WHITE)
    return INVALID_VAL;

  return colors[0] * 10 + colors[1];
}