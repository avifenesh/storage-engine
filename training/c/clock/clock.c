#include "clock.h"
#include <string.h>

static const char two_digit[60][2] = {
    {'0', '0'}, {'0', '1'}, {'0', '2'}, {'0', '3'}, {'0', '4'}, {'0', '5'},
    {'0', '6'}, {'0', '7'}, {'0', '8'}, {'0', '9'}, {'1', '0'}, {'1', '1'},
    {'1', '2'}, {'1', '3'}, {'1', '4'}, {'1', '5'}, {'1', '6'}, {'1', '7'},
    {'1', '8'}, {'1', '9'}, {'2', '0'}, {'2', '1'}, {'2', '2'}, {'2', '3'},
    {'2', '4'}, {'2', '5'}, {'2', '6'}, {'2', '7'}, {'2', '8'}, {'2', '9'},
    {'3', '0'}, {'3', '1'}, {'3', '2'}, {'3', '3'}, {'3', '4'}, {'3', '5'},
    {'3', '6'}, {'3', '7'}, {'3', '8'}, {'3', '9'}, {'4', '0'}, {'4', '1'},
    {'4', '2'}, {'4', '3'}, {'4', '4'}, {'4', '5'}, {'4', '6'}, {'4', '7'},
    {'4', '8'}, {'4', '9'}, {'5', '0'}, {'5', '1'}, {'5', '2'}, {'5', '3'},
    {'5', '4'}, {'5', '5'}, {'5', '6'}, {'5', '7'}, {'5', '8'}, {'5', '9'} };

static inline void set_time_str(clock_t *restrict clock, int hours,
                                 int minutes) {
  memcpy(clock->text, two_digit[hours], 2);
  clock->text[2] = ':';
  memcpy(clock->text + 3, two_digit[minutes], 2);
  clock->text[5] = '\0';
}

static inline int extract_hours(const clock_t *restrict clock) {
  return ((clock->text[0] - '0') * 10 + (clock->text[1] - '0'));
}

static inline int extract_minutes(const clock_t *restrict clock) {
  return ((clock->text[3] - '0') * 10 + (clock->text[4] - '0'));
}

static inline int add(int a, int b) { return a + b; }
static inline int subtract(int a, int b) { return a - b; }

static void apply_time_change(clock_t *restrict clock, int minutes,
                              int (*operation)(int, int)) {
  if (__builtin_expect(minutes == 0, 0))
    return;

  int hours = extract_hours(clock);
  int current_minutes = extract_minutes(clock);
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
  set_time_str(clock, hours, new_minutes);
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

  set_time_str(&clock, hours, minutes);

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
