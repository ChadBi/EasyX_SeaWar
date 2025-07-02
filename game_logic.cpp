// game_logic.cpp
#include "game_logic.h"
#include <algorithm>

void initialize_board(PlayerBoard& board) {
    // 使用默认构造的临时对象来重置board，代码更简洁
    board = PlayerBoard();
}

bool can_place_ship(const PlayerBoard& board, const Ship& ship) {
    if (ship.vertical) {
        if (ship.start.r + ship.size > GRID_SIZE) return false;
    }
    else {
        if (ship.start.c + ship.size > GRID_SIZE) return false;
    }

    // 检查船只周围是否有其他船
    for (int r_offset = -1; r_offset <= 1; ++r_offset) {
        for (int c_offset = -1; c_offset <= 1; ++c_offset) {
            for (int i = 0; i < ship.size; ++i) {
                int r = ship.start.r + (ship.vertical ? i : 0) + r_offset;
                int c = ship.start.c + (ship.vertical ? 0 : i) + c_offset;
                if (r >= 0 && r < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
                    if (board.grid[r][c] != CellState::EMPTY) return false;
                }
            }
        }
    }
    return true;
}

void place_ship_on_board(PlayerBoard& board, const Ship& ship) {
    board.ships.push_back(ship);
    for (int i = 0; i < ship.size; ++i) {
        int r = ship.start.r + (ship.vertical ? i : 0);
        int c = ship.start.c + (ship.vertical ? 0 : i);
        board.grid[r][c] = CellState::SHIP;
    }
}

// 设为static，限制其作用域在当前文件
static void check_and_update_sunk_ships(PlayerBoard& board) {
    for (auto& ship : board.ships) {
        if (!ship.is_sunk && ship.hits >= ship.size) { // 使用 >= 更安全
            ship.is_sunk = true;
            board.ships_sunk_count++;
            for (int i = 0; i < ship.size; ++i) {
                int r = ship.start.r + (ship.vertical ? i : 0);
                int c = ship.start.c + (ship.vertical ? 0 : i);
                board.grid[r][c] = CellState::SUNK;
            }
        }
    }
}

CellState process_shot(PlayerBoard& target_board, Point shot_coords) {
    int r = shot_coords.r;
    int c = shot_coords.c;

    if (target_board.grid[r][c] == CellState::SHIP) {
        target_board.grid[r][c] = CellState::HIT;
        for (auto& ship : target_board.ships) {
            if (ship.vertical) {
                if (c == ship.start.c && r >= ship.start.r && r < ship.start.r + ship.size) {
                    ship.hits++;
                    break;
                }
            }
            else {
                if (r == ship.start.r && c >= ship.start.c && c < ship.start.c + ship.size) {
                    ship.hits++;
                    break;
                }
            }
        }
        check_and_update_sunk_ships(target_board);
        return CellState::HIT;
    }
    else if (target_board.grid[r][c] == CellState::EMPTY) {
        target_board.grid[r][c] = CellState::MISS;
        return CellState::MISS;
    }
    return target_board.grid[r][c];
}

bool check_game_over(const PlayerBoard& board) {
    // 确保所有船都已放置
    if (board.ships.empty() || board.ships.size() != SHIP_SIZES.size()) return false;
    return board.ships_sunk_count == board.ships.size();
}