// common.h
#pragma once // 防止头文件被重复包含

#include <graphics.h>
#include <vector>
#include <string>

// --- 全局常量 ---
const int GRID_SIZE = 10;     // 棋盘尺寸 10x10
const int CELL_SIZE = 35;     // 每个格子的像素大小
const int BOARD_BORDER = 10;  // 棋盘边框宽度
const int WINDOW_WIDTH = 900;
const int WINDOW_HEIGHT = 500;
const std::vector<int> SHIP_SIZES = { 5, 4, 3, 3, 2 }; // 舰船大小配置

// --- 枚举定义 ---

// 游戏模式
enum class GameMode {
    PLAYER_VS_AI,
    PLAYER_VS_PLAYER
};

// 游戏当前状态
enum class GameState {
    MAIN_MENU,
    SHIP_PLACEMENT,
    PLAYER1_TURN,
    PLAYER2_TURN,
    AI_TURN,
    GAME_OVER
};

// 格子状态
enum class CellState {
    EMPTY, // 空
    SHIP,  // 有船
    HIT,   // 击中
    MISS,  // 未击中
    SUNK   // 击沉
};

// --- 结构体定义 ---

// 坐标点
struct Point {
    int r, c; // row, col
};

// 舰船
struct Ship {
    Point start;
    int size;
    bool vertical;
    int hits;
    bool is_sunk;
};

// 玩家棋盘
struct PlayerBoard {
    CellState grid[GRID_SIZE][GRID_SIZE];       // 自己的棋盘，记录船的位置
    std::vector<Ship> ships;
    int ships_sunk_count;
    PlayerBoard() : ships_sunk_count(0) {
        for (int r = 0; r < GRID_SIZE; ++r) {
            for (int c = 0; c < GRID_SIZE; ++c) {
                grid[r][c] = CellState::EMPTY;
            }
        }
    }
};