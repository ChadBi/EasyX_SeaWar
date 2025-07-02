// graphics.cpp
#include "graphics.h"
#include "game_logic.h"
#include <string>

IMAGE g_imgGradientBg;
// 辅助函数：根据格子状态获取颜色
// 设为 static，限制作用域
static COLORREF get_cell_color(CellState state) {
    switch (state) {
    case CellState::EMPTY: return RGB(173, 216, 230);
    case CellState::SHIP:  return DARKGRAY; // <--- 修正颜色
    case CellState::HIT:   return RGB(255, 165, 0);
    case CellState::MISS:  return WHITE;
    case CellState::SUNK:  return RED;
    default:               return BLACK;
    }
}

void create_gradient_background() {
    // 将绘图目标切换到我们的离屏 IMAGE 对象上
    SetWorkingImage(&g_imgGradientBg);

    // 这部分代码和之前 draw_background 的一样
    COLORREF start_color = RGB(15, 32, 72);
    COLORREF end_color = RGB(75, 125, 190);
    int r1 = GetRValue(start_color), g1 = GetGValue(start_color), b1 = GetBValue(start_color);
    int r2 = GetRValue(end_color), g2 = GetGValue(end_color), b2 = GetBValue(end_color);
    for (int y = 0; y < WINDOW_HEIGHT; ++y) {
        double ratio = (double)y / WINDOW_HEIGHT;
        int r = static_cast<int>(r1 + (r2 - r1) * ratio);
        int g = static_cast<int>(g1 + (g2 - g1) * ratio);
        int b = static_cast<int>(b1 + (b2 - b1) * ratio);
        setlinecolor(RGB(r, g, b));
        line(0, y, WINDOW_WIDTH, y);
    }

    // !!重要!! 将绘图目标切换回默认的窗口
    SetWorkingImage(NULL);
}

void draw_main_menu(int selected_item) {
    //cleardevice();
    settextstyle(60, 0, L"Impact");
    settextcolor(DARKGRAY);
    outtextxy(340, 80, L"BATTLESHIP");

    const wchar_t* items[] = { L"人机对战", L"双人对战", L"退出游戏" };
    for (int i = 0; i < 3; ++i) {
        if (i == selected_item) {
            setfillcolor(RGB(100, 149, 237)); // 选中颜色
        }
        else {
            setfillcolor(RGB(176, 196, 222)); // 普通颜色
        }
        solidroundrect(300, 200 + i * 80, 600, 260 + i * 80, 10, 10);
        setbkmode(TRANSPARENT);
        settextcolor(WHITE);
        settextstyle(32, 0, L"微软雅黑");
        outtextxy(390, 215 + i * 80, items[i]);
    }
}

void draw_game_board(int x, int y, const PlayerBoard& board, bool show_ships) {
    for (int r = 0; r < GRID_SIZE; ++r) {
        for (int c = 0; c < GRID_SIZE; ++c) {
            int px = x + c * CELL_SIZE;
            int py = y + r * CELL_SIZE;

            CellState state_to_draw = board.grid[r][c];
            if (!show_ships && state_to_draw == CellState::SHIP) {
                state_to_draw = CellState::EMPTY;
            }

            setfillcolor(get_cell_color(state_to_draw));
            solidrectangle(px, py, px + CELL_SIZE, py + CELL_SIZE);
            setlinecolor(BLACK);
            rectangle(px, py, px + CELL_SIZE, py + CELL_SIZE);
        }
    }
}

void draw_placement_screen(const PlayerBoard& board, const Ship& current_ship_preview, bool placement_valid) {
    //cleardevice();
    settextstyle(24, 0, L"微软雅黑");
    settextcolor(BLACK);
    outtextxy(100, 20, L"请放置你的舰船 (右键旋转, 左键放置)");

    draw_game_board(100, 60, board, true);

    // 绘制预览船只
    setfillcolor(placement_valid ? GREEN : RED);
    for (int i = 0; i < current_ship_preview.size; ++i) {
        int r = current_ship_preview.start.r + (current_ship_preview.vertical ? i : 0);
        int c = current_ship_preview.start.c + (current_ship_preview.vertical ? 0 : i);
        if (r < GRID_SIZE && c < GRID_SIZE) {
            int px = 100 + c * CELL_SIZE;
            int py = 60 + r * CELL_SIZE;
            solidrectangle(px, py, px + CELL_SIZE, py + CELL_SIZE);
        }
    }
}

void draw_game_interface(const PlayerBoard& p1, const PlayerBoard& p2, GameState current_state, GameMode mode) {
    //cleardevice();

    int p1_board_x = 50;
    int p2_board_x = WINDOW_WIDTH - p1_board_x - GRID_SIZE * CELL_SIZE;
    int board_y = 100;

    settextstyle(24, 0, L"微软雅黑");
    settextcolor(BLACK);
    // 优化PVP模式下的标题
    if (mode == GameMode::PLAYER_VS_PLAYER && (current_state == GameState::PLAYER2_TURN || current_state == GameState::PLAYER1_TURN)) {
        outtextxy(p1_board_x, 60, L"玩家1 的棋盘");
        outtextxy(p2_board_x, 60, L"玩家2 的棋盘");
    }
    else {
        outtextxy(p1_board_x, 60, L"你的棋盘");
        outtextxy(p2_board_x, 60, L"对手棋盘");
    }

    bool show_p1_ships = (mode == GameMode::PLAYER_VS_AI || current_state != GameState::PLAYER2_TURN);
    bool show_p2_ships = (mode == GameMode::PLAYER_VS_PLAYER && current_state != GameState::PLAYER1_TURN);

    // 在PVP模式下，轮到谁，谁的船就隐藏
    if (mode == GameMode::PLAYER_VS_PLAYER) {
        draw_game_board(p1_board_x, board_y, p1, current_state != GameState::PLAYER2_TURN);
        draw_game_board(p2_board_x, board_y, p2, current_state != GameState::PLAYER1_TURN);
    }
    else {
        draw_game_board(p1_board_x, board_y, p1, true);
        draw_game_board(p2_board_x, board_y, p2, false);
    }

    settextstyle(30, 0, L"Impact");
    std::wstring status_text;
    switch (current_state) {
    case GameState::PLAYER1_TURN: status_text = L"玩家1 回合 (攻击右侧)"; break;
        // 优化PVP提示
    case GameState::PLAYER2_TURN: status_text = (mode == GameMode::PLAYER_VS_AI) ? L"" : L"玩家2 回合 (攻击左侧)"; break;
    case GameState::AI_TURN: status_text = L"AI 正在思考..."; break;
    case GameState::GAME_OVER:
        if (check_game_over(p2)) status_text = L"玩家1 获胜!";
        else status_text = (mode == GameMode::PLAYER_VS_AI) ? L"AI 获胜!" : L"玩家2 获胜!";
        break;
    }
    outtextxy(WINDOW_WIDTH / 2 - 120, 20, status_text.c_str());
}

Point get_grid_click(int mouse_x, int mouse_y, int grid_start_x, int grid_start_y) {
    if (mouse_x < grid_start_x || mouse_x > grid_start_x + GRID_SIZE * CELL_SIZE ||
        mouse_y < grid_start_y || mouse_y > grid_start_y + GRID_SIZE * CELL_SIZE) {
        return { -1, -1 }; // 点击在棋盘外
    }
    int c = (mouse_x - grid_start_x) / CELL_SIZE;
    int r = (mouse_y - grid_start_y) / CELL_SIZE;
    return { r, c };
}

void draw_background() {
    putimage(0, 0, &g_imgGradientBg);
}