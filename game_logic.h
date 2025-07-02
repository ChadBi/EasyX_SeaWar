// game_logic.h
#pragma once
#include "common.h"

void initialize_board(PlayerBoard& board);
bool can_place_ship(const PlayerBoard& board, const Ship& ship);
void place_ship_on_board(PlayerBoard& board, const Ship& ship);
CellState process_shot(PlayerBoard& target_board, Point shot_coords);
bool check_game_over(const PlayerBoard& board);