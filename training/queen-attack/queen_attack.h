#ifndef QUEEN_ATTACK_H
#define QUEEN_ATTACK_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum { CAN_NOT_ATTACK, CAN_ATTACK, INVALID_POSITION } attack_status_t;

typedef struct {
  uint8_t row;
  uint8_t column;
} position_t;

attack_status_t can_attack(position_t queen_1, position_t queen_2);
bool out_of_board(position_t queen);
bool same_position(position_t queen_1, position_t queen_2);
bool on_same_row_or_column(position_t queen_1, position_t queen_2);
bool on_same_diagonal(position_t queen_1, position_t queen_2);

#endif
