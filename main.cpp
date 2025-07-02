#include <conio.h>
#include "common.h"
#include "graphics.h"
#include "game_logic.h"
#include "ai_player.h"

// --- 函数声明 ---
// 从 graphics.cpp 引入的函数
void draw_background();

// 本文件内的函数
void main_menu_loop();
void game_loop(GameMode mode);
void placement_phase(PlayerBoard& board, const std::wstring& player_name);
void show_transition_screen(const std::wstring& text);
void create_gradient_background();


int main() {
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    g_imgGradientBg.Resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    create_gradient_background();
    // 开启批量绘图模式，防止画面闪烁。包裹整个程序生命周期。
    BeginBatchDraw();

    // 进入主菜单循环
    main_menu_loop();

    // 结束批量绘图
    EndBatchDraw();
    closegraph();
    return 0;
}

/**
 * @brief 显示一个全屏的过渡信息，等待用户按键继续。
 * @param text 要显示的提示文本。
 */
void show_transition_screen(const std::wstring& text) {
    draw_background(); // 绘制渐变色背景
    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle(30, 0, L"微软雅黑");
    // 计算文本居中位置
    int text_w = textwidth(text.c_str());
    outtextxy((WINDOW_WIDTH - text_w) / 2, WINDOW_HEIGHT / 2 - 15, text.c_str());
    FlushBatchDraw();
    //_getch(); // 等待任意键按下
    ExMessage tmp;
    while (true) {
		tmp = getmessage(EX_KEY);
        if (tmp.message == WM_KEYDOWN) {
            break; // 只要有键盘点击事件就退出
		}
    }

}

/**
 * @brief 主菜单的事件循环。
 */
void main_menu_loop() {
    int selected_item = 0;
    ExMessage msg;
    while (true) {
        // 绘制背景和菜单
        draw_background();
        draw_main_menu(selected_item);
        FlushBatchDraw();

        // 检查鼠标消息
        if (peekmessage(&msg, EX_MOUSE)) {
            // 鼠标悬停检测
            if (msg.message == WM_MOUSEMOVE) {
                if (msg.x > 300 && msg.x < 600) {
                    if (msg.y > 200 && msg.y < 260) selected_item = 0;
                    else if (msg.y > 280 && msg.y < 340) selected_item = 1;
                    else if (msg.y > 360 && msg.y < 420) selected_item = 2;
                    else selected_item = -1;
                }
                else {
                    selected_item = -1;
                }
            }
            // 鼠标点击处理
            if (msg.message == WM_LBUTTONDOWN && selected_item != -1) {
                switch (selected_item) {
                case 0: // 人机对战
                    game_loop(GameMode::PLAYER_VS_AI);
                    break;
                case 1: // 双人对战
                    game_loop(GameMode::PLAYER_VS_PLAYER);
                    break;
                case 2: // 退出游戏
                    return; // 退出循环，结束程序
                }
            }
        }
        Sleep(10);
    }
}

/**
 * @brief 游戏主循环，处理从放置到对战结束的全过程。
 * @param mode 游戏模式 (人机 或 双人)。
 */
void game_loop(GameMode mode) {
    PlayerBoard p1_board, p2_board;
    AIPlayer ai;

    // --- 1. 舰船放置阶段 ---
    placement_phase(p1_board, L"玩家1");

    if (mode == GameMode::PLAYER_VS_PLAYER) {
        show_transition_screen(L"玩家1 放置完毕，请玩家2准备 (按任意键)");
        placement_phase(p2_board, L"玩家2");
        show_transition_screen(L"放置完毕，按任意键开始对战！");
    }
    else { // 人机模式
        ai.place_ships(p2_board);
    }

    // --- 2. 对战阶段 ---
    GameState current_state = GameState::PLAYER1_TURN;
    ExMessage msg;

    while (true) {
        // 检查游戏是否结束
        if (check_game_over(p1_board) || check_game_over(p2_board)) {
            current_state = GameState::GAME_OVER;
        }

        // 统一绘制背景和游戏界面
        draw_background();
        draw_game_interface(p1_board, p2_board, current_state, mode);
        FlushBatchDraw();

        // 如果游戏结束，显示结果几秒后退出循环
        if (current_state == GameState::GAME_OVER) {
            Sleep(3000);
            break;
        }

        bool turn_processed = false;

        // --- 3. 根据当前回合处理玩家或AI的输入 ---
        switch (current_state) {
        case GameState::PLAYER1_TURN:
            if (peekmessage(&msg, EX_MOUSE) && msg.message == WM_LBUTTONDOWN) {
                int p2_board_x = WINDOW_WIDTH - 50 - GRID_SIZE * CELL_SIZE;
                Point shot = get_grid_click(msg.x, msg.y, p2_board_x, 100);
                if (shot.r != -1 && p2_board.grid[shot.r][shot.c] < CellState::HIT) {
                    if (process_shot(p2_board, shot) != CellState::HIT) {
                        turn_processed = true;
                    }
                }
            }
            break;

        case GameState::PLAYER2_TURN: // 仅在PVP模式下有效
            if (mode == GameMode::PLAYER_VS_PLAYER && peekmessage(&msg, EX_MOUSE) && msg.message == WM_LBUTTONDOWN) {
                int p1_board_x = 50;
                Point shot = get_grid_click(msg.x, msg.y, p1_board_x, 100);
                if (shot.r != -1 && p1_board.grid[shot.r][shot.c] < CellState::HIT) {
                    if (process_shot(p1_board, shot) != CellState::HIT) {
                        turn_processed = true;
                    }
                }
            }
            break;

        case GameState::AI_TURN: // 仅在PVE模式下有效
            if (mode == GameMode::PLAYER_VS_AI) {
                Sleep(500); // 模拟AI思考

                // 1. AI根据当前状态决定射击点
                Point shot = ai.make_shot(p1_board.grid);

                // 2. 处理射击，获取结果
                CellState result = process_shot(p1_board, shot);

                // 3. 将射击结果反馈给AI，让它更新自己的状态
                ai.report_shot_result(shot, result);

                // 旧的逻辑：只判断是否交换回合
                // if (process_shot(p1_board, shot) != CellState::HIT) {
                //     turn_processed = true;
                // }

                // 新的逻辑：如果没击中或击沉了，就交换回合
                // 注意：process_shot返回的是更新前的状态，但p1_board.grid已经被更新了
                // 我们直接检查更新后的格子状态
                if (p1_board.grid[shot.r][shot.c] == CellState::MISS) {
                    turn_processed = true;
                }

                // 检查是否因为这一击导致游戏结束（所有船都被击沉）
                if (check_game_over(p1_board)) {
                    // 游戏结束，不需要交换回合
                }
                else if (p1_board.grid[shot.r][shot.c] == CellState::SUNK) {
                    // 如果击沉了一艘船，AI也需要重置状态，但可能连续攻击
                    // 这里为了简化，我们让AI击沉后也交换回合，表现更像人类玩家
                    // 你也可以让AI在击沉后立即进行下一次HUNTING射击
                    turn_processed = true;
                }
            }
            break;
        }

        // --- 4. 如果回合结束，则切换玩家 ---
        if (turn_processed) {
            if (current_state == GameState::PLAYER1_TURN) {
                current_state = (mode == GameMode::PLAYER_VS_AI) ? GameState::AI_TURN : GameState::PLAYER2_TURN;
                if (mode == GameMode::PLAYER_VS_PLAYER) {
                    show_transition_screen(L"轮到玩家2，请玩家1回避 (按任意键)");
                }
            }
            else { // AI_TURN 或 PLAYER2_TURN 结束
                current_state = GameState::PLAYER1_TURN;
                if (mode == GameMode::PLAYER_VS_PLAYER) {
                    show_transition_screen(L"轮到玩家1，请玩家2回避 (按任意键)");
                }
            }
        }
        Sleep(1);
    }
}

/**
 * @brief 处理单个玩家的舰船放置阶段。
 * @param board 要放置舰船的玩家棋盘。
 * @param player_name 当前玩家的名称，用于显示提示。
 */
void placement_phase(PlayerBoard& board, const std::wstring& player_name) {
    initialize_board(board);
    int current_ship_idx = 0;
    Ship preview_ship;
    preview_ship.vertical = false;
    preview_ship.hits = 0;
    preview_ship.is_sunk = false;

    ExMessage msg;
    while (current_ship_idx < SHIP_SIZES.size()) {
        preview_ship.size = SHIP_SIZES[current_ship_idx];

        // 持续获取鼠标和键盘消息
        if (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
            // 获取鼠标在网格中的位置
            Point grid_pos = get_grid_click(msg.x, msg.y, 100, 60);
            if (grid_pos.r != -1) {
                preview_ship.start = grid_pos;
            }

            // 右键旋转
            if (msg.message == WM_RBUTTONDOWN) {
                preview_ship.vertical = !preview_ship.vertical;
            }

            // 左键放置
            bool is_valid = can_place_ship(board, preview_ship);
            if (msg.message == WM_LBUTTONDOWN && grid_pos.r != -1 && is_valid) {
                place_ship_on_board(board, preview_ship);
                current_ship_idx++;
            }
        }

        // 绘制背景和放置界面
        draw_background();
        bool is_valid_now = can_place_ship(board, preview_ship);
        draw_placement_screen(board, preview_ship, is_valid_now);

        // 动态显示提示信息
        std::wstring hint = player_name + L", 请放置你的 " + std::to_wstring(preview_ship.size) + L" 格舰船";
        setbkmode(TRANSPARENT);
        settextcolor(WHITE);
        settextstyle(24, 0, L"微软雅黑");
        outtextxy(100, 420, hint.c_str());

        FlushBatchDraw();
        Sleep(10);
    }
}