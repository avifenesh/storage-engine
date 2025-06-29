#include "clock.h"

#define EXTRACT_HOURS(clock)                                                   \
  ((clock->text[0] - '0') * 10 + (clock->text[1] - '0'))

#define EXTRACT_MINUTES(clock)                                                 \
  ((clock->text[3] - '0') * 10 + (clock->text[4] - '0'))

#define SET_TIME(clock, h, m)                                                  \
  do {                                                                         \
    clock->text[0] = '0' + (h) / 10;                                           \
    clock->text[1] = '0' + (h) % 10;                                           \
    clock->text[3] = '0' + (m) / 10;                                           \
    clock->text[4] = '0' + (m) % 10;                                           \
  } while (0)

static inline int add(int a, int b) { return a + b; }
static inline int subtract(int a, int b) { return a - b; }

static void apply_time_change(clock_t *restrict clock, int minutes,
                              int (*operation)(int, int)) {
  if (__builtin_expect(minutes == 0, 0))
    return;

  int hours = EXTRACT_HOURS(clock);
  int current_minutes = EXTRACT_MINUTES(clock);
  int new_minutes = operation(current_minutes, minutes);

  if (__builtin_expect(new_minutes >= 60, 0)) {
    hours += new_minutes / 60;
    new_minutes %= 60;
  } else if (__builtin_expect(new_minutes < 0, 0)) {
    int hour_decrement = (-new_minutes + 59) / 60;
    hours -= hour_decrement;
    new_minutes += hour_decrement * 60;
  }

  hours = ((hours % 24) + 24) % 24;
  SET_TIME(clock, hours, new_minutes);
}

bool clock_is_equal(clock_t a, clock_t b) {
  return !(((a.text[0] ^ b.text[0]) | (a.text[1] ^ b.text[1]) |
            (a.text[3] ^ b.text[3]) | (a.text[4] ^ b.text[4])));
}

clock_t clock_create(int hour, int minute) {
  clock_t clock;

  int total_minutes = hour * 60 + minute;
  total_minutes = (total_minutes % 1440 + 1440) % 1440;

  int hours = total_minutes / 60;
  int minutes = total_minutes - hours * 60;

  clock.text[0] = '0' + hours / 10;
  clock.text[1] = '0' + hours % 10;
  clock.text[2] = ':';
  clock.text[3] = '0' + minutes / 10;
  clock.text[4] = '0' + minutes % 10;
  clock.text[5] = '\0';

  return clock;
}

clock_t clock_add(clock_t clock, int minute_add) {
  apply_time_change(&clock, minute_add, add);
  return clock;
}

clock_t clock_subtract(clock_t clock, int minute_subtract) {
  apply_time_change(&clock, minute_subtract, subtract);
  return clock;
}