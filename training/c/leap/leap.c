#include "leap.h"
bool mod_is_zero(int year, int mod_of) { return year % mod_of == 0; }
bool leap_year(int year) {
  return mod_is_zero(year, 400)   ? true
         : mod_is_zero(year, 100) ? false
         : mod_is_zero(year, 4)   ? true
                                  : false;
}