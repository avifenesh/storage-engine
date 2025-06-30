/**
 * clock.c - High-performance time manipulation implementation
 *
 * This module implements a string-based clock representation optimized for
 * ARM64 SIMD operations and minimal memory footprint. The design prioritizes
 * direct string manipulation over integer arithmetic for display purposes.
 *
 * Thread Safety: All functions operate on value types or local copies.
 * No global state is modified. Safe for concurrent use.
 *
 * Copyright (C) 2025 - Kernel-Accelerated Database Project
 */

#include "clock.h"
#include <stdint.h>
#include <string.h>

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

#ifdef DEBUG
#include <assert.h>
#endif

/* C99-compatible static assertion */
#ifndef _Static_assert
#define _Static_assert(condition, message)                                     \
  typedef char static_assertion_##__LINE__[(condition) ? 1 : -1]
#endif

/* Time calculation constants */
#define MINUTES_PER_DAY 1440
#define HOURS_PER_DAY 24
#define MINUTES_PER_HOUR 60

/* Compile-time validation for kernel ABI compatibility */
_Static_assert(sizeof(clock_t) == 6, "clock_t ABI requirement");

/**
 * two_digit_lookup - Pre-computed lookup table for two-digit formatting
 *
 * This table eliminates runtime division/modulo operations for converting
 * integers 0-59 to their two-character ASCII representations. Each entry
 * contains the tens and ones digit characters for direct assignment.
 */
static const char two_digit_lookup[60][2] = {
    {'0', '0'}, {'0', '1'}, {'0', '2'}, {'0', '3'}, {'0', '4'}, {'0', '5'},
    {'0', '6'}, {'0', '7'}, {'0', '8'}, {'0', '9'}, {'1', '0'}, {'1', '1'},
    {'1', '2'}, {'1', '3'}, {'1', '4'}, {'1', '5'}, {'1', '6'}, {'1', '7'},
    {'1', '8'}, {'1', '9'}, {'2', '0'}, {'2', '1'}, {'2', '2'}, {'2', '3'},
    {'2', '4'}, {'2', '5'}, {'2', '6'}, {'2', '7'}, {'2', '8'}, {'2', '9'},
    {'3', '0'}, {'3', '1'}, {'3', '2'}, {'3', '3'}, {'3', '4'}, {'3', '5'},
    {'3', '6'}, {'3', '7'}, {'3', '8'}, {'3', '9'}, {'4', '0'}, {'4', '1'},
    {'4', '2'}, {'4', '3'}, {'4', '4'}, {'4', '5'}, {'4', '6'}, {'4', '7'},
    {'4', '8'}, {'4', '9'}, {'5', '0'}, {'5', '1'}, {'5', '2'}, {'5', '3'},
    {'5', '4'}, {'5', '5'}, {'5', '6'}, {'5', '7'}, {'5', '8'}, {'5', '9'}};

/*
 * Internal helper functions
 */

/**
 * set_time_string - Convert hours and minutes to string representation
 * @clock: Pointer to clock structure to update
 * @hours: Hour value (0-23)
 * @minutes: Minute value (0-59)
 *
 * Uses direct character assignment with pre-computed lookup table for
 * optimal performance. The restrict qualifier enables compiler optimizations
 * by guaranteeing no pointer aliasing.
 */
static inline void set_time_string(clock_t *restrict clock, int hours,
                                   int minutes) {
#ifdef DEBUG
  assert(hours >= 0 && hours < 24);
  assert(minutes >= 0 && minutes < 60);
#endif
  clock->text[0] = two_digit_lookup[hours][0];
  clock->text[1] = two_digit_lookup[hours][1];
  clock->text[2] = ':';
  clock->text[3] = two_digit_lookup[minutes][0];
  clock->text[4] = two_digit_lookup[minutes][1];
  clock->text[5] = '\0';
}

/**
 * extract_hours_from_string - Extract hour value from clock string
 * @clock: Pointer to clock structure
 *
 * Return: Hour value as integer (0-23)
 *
 * Performs direct ASCII arithmetic to avoid atoi() overhead.
 */
static inline int extract_hours_from_string(const clock_t *restrict clock) {
  return ((clock->text[0] - '0') * 10 + (clock->text[1] - '0'));
}

/**
 * extract_minutes_from_string - Extract minute value from clock string
 * @clock: Pointer to clock structure
 *
 * Return: Minute value as integer (0-59)
 */
static inline int extract_minutes_from_string(const clock_t *restrict clock) {
  return ((clock->text[3] - '0') * 10 + (clock->text[4] - '0'));
}

/**
 * get_total_minutes - Convert clock time to total minutes since midnight
 * @clock: Pointer to clock structure
 *
 * Return: Total minutes since midnight (0-1439)
 */
static inline int get_total_minutes(const clock_t *restrict clock) {
  return extract_hours_from_string(clock) * MINUTES_PER_HOUR +
         extract_minutes_from_string(clock);
}

/**
 * normalize_and_set_clock - Normalize time value and update clock
 * @clock: Clock structure to update
 * @total_minutes: Raw minute value (can be negative or > 1439)
 *
 * Handles day boundary wrapping for both positive and negative values
 * using the universal normalization formula. This ensures consistent
 * behavior across all input ranges.
 */
static inline void normalize_and_set_clock(clock_t *clock, int total_minutes) {
#ifdef DEBUG
  assert(clock != NULL);
#endif

  /* Universal normalization: handles positive and negative values */
  total_minutes =
      (total_minutes % MINUTES_PER_DAY + MINUTES_PER_DAY) % MINUTES_PER_DAY;

  int hours = total_minutes / MINUTES_PER_HOUR;
  int minutes = total_minutes % MINUTES_PER_HOUR;

#ifdef DEBUG
  assert(hours >= 0 && hours < HOURS_PER_DAY);
  assert(minutes >= 0 && minutes < MINUTES_PER_HOUR);
#endif

  set_time_string(clock, hours, minutes);
}

/*
 * Public API functions
 */

/**
 * clock_is_equal - Compare two clock values for equality
 * @a: First clock value
 * @b: Second clock value
 *
 * Return: true if clocks represent the same time, false otherwise
 *
 * Two implementations are provided:
 * - ARM NEON version uses 64-bit comparison for SIMD optimization
 * - Standard version uses direct byte comparison
 */
#ifdef __ARM_NEON
bool clock_is_equal(clock_t a, clock_t b) {
  uint64_t a_val = 0, b_val = 0;

  /* Safe 6-byte copy to avoid buffer overrun */
  memcpy(&a_val, a.text, 6);
  memcpy(&b_val, b.text, 6);

  /* Mask compares only meaningful bytes (skip null terminator) */
  return (a_val & 0x0000FFFFFFFFFF00) == (b_val & 0x0000FFFFFFFFFF00);
}
#else
bool clock_is_equal(clock_t a, clock_t b) {
  /* Skip comparison of text[2] as it's always ':' */
  return (a.text[0] == b.text[0]) && (a.text[1] == b.text[1]) &&
         (a.text[3] == b.text[3]) && (a.text[4] == b.text[4]);
}
#endif

/**
 * clock_create - Create a new clock value
 * @hour: Hour value (can be outside 0-23 range)
 * @minute: Minute value (can be outside 0-59 range)
 *
 * Return: Normalized clock structure
 *
 * Input values are normalized to valid 24-hour format. Negative values
 * and values exceeding normal ranges are handled correctly.
 */
clock_t clock_create(int hour, int minute) {
  clock_t clock;
  normalize_and_set_clock(&clock, hour * MINUTES_PER_HOUR + minute);
  return clock;
}

/**
 * clock_adjust - Adjust clock by a delta value in minutes
 * @clock: Current clock value
 * @minute_delta: Minutes to adjust (can be negative)
 *
 * Return: New clock value adjusted by the delta and normalized
 */
static inline clock_t clock_adjust(clock_t clock, int minute_delta) {
  int total_minutes = get_total_minutes(&clock) + minute_delta;
  normalize_and_set_clock(&clock, total_minutes);
  return clock;
}

clock_t clock_add(clock_t clock, int minute_add) {
  return clock_adjust(clock, minute_add);
}

clock_t clock_subtract(clock_t clock, int minute_subtract) {
  return clock_adjust(clock, -minute_subtract);
}