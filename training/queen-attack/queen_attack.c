#include "queen_attack.h"

attack_status_t can_attack(position_t queen_1, position_t queen_2) {
  if (out_of_board(queen_1) || out_of_board(queen_2))
    return INVALID_POSITION;

  if (same_position(queen_1, queen_2))
    return INVALID_POSITION;

  if (on_same_row_or_column(queen_1, queen_2) ||
      on_same_diagonal(queen_1, queen_2))
    return CAN_ATTACK;

  return CAN_NOT_ATTACK;
}

bool out_of_board(position_t queen) {
  return (queen.column > 7 || queen.row > 7);
}

bool same_position(position_t queen_1, position_t queen_2) {
  return (queen_1.column == queen_2.column && queen_1.row == queen_2.row);
}

bool on_same_row_or_column(position_t queen_1, position_t queen_2) {
  return (queen_1.column == queen_2.column || queen_1.row == queen_2.row);
}

bool on_same_diagonal(position_t queen_1, position_t queen_2) {
  return (abs(queen_1.column - queen_2.column) ==
          abs(queen_1.row - queen_2.row));
}