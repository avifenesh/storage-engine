#include "space_age.h"

// Precalculated reciprocals to avoid division
// For the CPU division is expensive, and better to avoid.
// While the diff is neglegedable in most cases, the goal of the learning is databse and kernel programming and each cpu cycle matter.
// earth_year_in_secondes = 31557600
// Planet X * earth year secondes = planet X year in secondes
// 1.0f / planet X year in secondes = desired multiplyer to get the age
const float orbital_reciprocals[] = {
    1.315e-7f,  // MERCURY
    5.151e-8f,  // VENUS
    3.169e-8f,  // EARTH
    1.685e-8f,  // MARS
    2.671e-9f,  // JUPITER
    1.076e-9f,  // SATURN
    3.771e-10f, // URANUS
    1.923e-10f  // NEPTUNE
};

float age(planet_t planet, int64_t seconds){
    if(seconds < 0 || planet < 0 || planet > 7)
        return INVALID_INPUT;
    if(seconds == 0)
        return 0.0f;
    return seconds * orbital_reciprocals[planet];
}