#ifndef RESISTOR_COLOR_H
#define RESISTOR_COLOR_H

#define RESISTOR_COLORS                                                        \
  X(BLACK, 0)                                                                  \
  X(BROWN, 1)                                                                  \
  X(RED, 2)                                                                    \
  X(ORANGE, 3)                                                                 \
  X(YELLOW, 4)                                                                 \
  X(GREEN, 5)                                                                  \
  X(BLUE, 6)                                                                   \
  X(VIOLET, 7)                                                                 \
  X(GREY, 8)                                                                   \
  X(WHITE, 9)

#define X(name, value) name = value,
typedef enum { RESISTOR_COLORS } resistor_band_t;
#undef X
unsigned char color_code(resistor_band_t color);
const resistor_band_t *colors(void);

#endif
