// ai_player.h
#pragma once
#include "common.h"
#include <vector>

// 定义AI的两种工作状态
enum class AIState {
    HUNTING,    // 搜索模式：随机寻找目标
    TARGETING   // 摧毁模式：锁定并摧毁已发现的目标
};

class AIPlayer {
public:
    AIPlayer();
    void place_ships(PlayerBoard& board);
    Point make_shot(const CellState opponent_view[GRID_SIZE][GRID_SIZE]);

    // 新增一个函数，用于接收上次射击的结果，并据此更新AI的状态
    void report_shot_result(Point shot, CellState result);

    void reset();

private:
    AIState m_state;                // AI当前的状态 (使用 m_ 前缀是成员变量的好习惯)
    std::vector<Point> m_target_hits; // 在摧毁模式下，存储已击中的船体部分坐标
};