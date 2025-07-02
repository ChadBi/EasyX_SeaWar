// graphics.h
#pragma once
#include "common.h"

// 函数声明
void create_gradient_background();
void draw_background(); // 声明绘制背景的函数
void draw_main_menu(int selected_item);
void draw_game_board(int x, int y, const PlayerBoard& board, bool show_ships);
void draw_placement_screen(const PlayerBoard& board, const Ship& current_ship_preview, bool placement_valid);
void draw_game_interface(const PlayerBoard& p1, const PlayerBoard& p2, GameState current_state, GameMode mode);
Point get_grid_click(int mouse_x, int mouse_y, int grid_start_x, int grid_start_y);
extern IMAGE g_imgGradientBg;