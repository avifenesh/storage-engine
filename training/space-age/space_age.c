#include "space_age.h"
#define SECONDS_PER_EARTH_YEAR (365.25f * 24 * 60 * 60)

static const float ORBITAL_RECIPROCALS[] = {
    [MERCURY] = 1.0f / (0.2408467f * SECONDS_PER_EARTH_YEAR),
    [VENUS]   = 1.0f / (0.61519726f * SECONDS_PER_EARTH_YEAR),
    [EARTH]   = 1.0f / (1.0f * SECONDS_PER_EARTH_YEAR),
    [MARS]    = 1.0f / (1.8808158f * SECONDS_PER_EARTH_YEAR),
    [JUPITER] = 1.0f / (11.862615f * SECONDS_PER_EARTH_YEAR),
    [SATURN]  = 1.0f / (29.447498f * SECONDS_PER_EARTH_YEAR),
    [URANUS]  = 1.0f / (84.016846f * SECONDS_PER_EARTH_YEAR),
    [NEPTUNE] = 1.0f / (164.79132f * SECONDS_PER_EARTH_YEAR)
};

float age(planet_t planet, int64_t seconds) {
  if (seconds < 0 || planet < 0 || planet > 7)
    return INVALID_INPUT;
  return seconds * ORBITAL_RECIPROCALS[planet];
}