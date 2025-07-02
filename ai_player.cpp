// ai_player.cpp
#include "ai_player.h"
#include "game_logic.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <vector>

AIPlayer::AIPlayer() {
    srand(static_cast<unsigned int>(time(nullptr)));
    reset();
}

// 重置AI状态，在每局游戏开始时调用
void AIPlayer::reset() {
    m_state = AIState::HUNTING; // 初始状态为搜索
    m_target_hits.clear();      // 清空目标列表
}

// 放置舰船的逻辑保持不变
void AIPlayer::place_ships(PlayerBoard& board) {
    initialize_board(board);
    for (int size : SHIP_SIZES) {
        Ship ship;
        ship.size = size;
        ship.hits = 0;
        ship.is_sunk = false;
        do {
            ship.start = { rand() % GRID_SIZE, rand() % GRID_SIZE };
            ship.vertical = (rand() % 2 == 0);
        } while (!can_place_ship(board, ship));
        place_ship_on_board(board, ship);
    }
}

/**
 * @brief 根据当前状态决定下一次射击的位置。
 */
Point AIPlayer::make_shot(const CellState opponent_view[GRID_SIZE][GRID_SIZE]) {
    if (m_state == AIState::TARGETING) {
        // --- 摧毁模式逻辑 ---
        std::vector<Point> candidates; // 候选攻击点

        // 如果只有一个击中点，攻击它的四周
        if (m_target_hits.size() == 1) {
            Point hit = m_target_hits[0];
            candidates.push_back({ hit.r + 1, hit.c });
            candidates.push_back({ hit.r - 1, hit.c });
            candidates.push_back({ hit.r, hit.c + 1 });
            candidates.push_back({ hit.r, hit.c - 1 });
        }
        // 如果有两个或更多的击中点，说明已确定船的方向，攻击延长线
        else {
            // 对已击中点进行排序，方便找到船的两端
            std::sort(m_target_hits.begin(), m_target_hits.end(), [](const Point& a, const Point& b) {
                if (a.r != b.r) return a.r < b.r;
                return a.c < b.c;
                });

            Point first = m_target_hits.front();
            Point last = m_target_hits.back();

            // 判断是水平还是垂直
            if (first.r == last.r) { // 水平
                candidates.push_back({ first.r, first.c - 1 }); // 左侧延长线
                candidates.push_back({ last.r, last.c + 1 });   // 右侧延长线
            }
            else { // 垂直
                candidates.push_back({ first.r - 1, first.c }); // 上方延长线
                candidates.push_back({ last.r + 1, last.c });   // 下方延长线
            }
        }

        // 从候选点中筛选出有效且未攻击过的点
        for (Point p : candidates) {
            if (p.r >= 0 && p.r < GRID_SIZE && p.c >= 0 && p.c < GRID_SIZE &&
                opponent_view[p.r][p.c] < CellState::HIT) {
                return p; // 找到第一个有效的点，立即攻击
            }
        }

        // 如果延长线上的点都无效（比如船靠边或已被打过），则退回搜索模式以防死循环
        m_state = AIState::HUNTING;
    }

    // --- 搜索模式逻辑 ---
    // 随机射击一个没有打过的点
    Point shot;
    do {
        shot = { rand() % GRID_SIZE, rand() % GRID_SIZE };
    } while (opponent_view[shot.r][shot.c] >= CellState::HIT);
    return shot;
}


/**
 * @brief 接收上一次射击的结果，并更新AI的内部状态。
 * @param shot 上次射击的坐标。
 * @param result 上次射击的结果 (HIT, MISS, SUNK)。
 */
void AIPlayer::report_shot_result(Point shot, CellState result) {
    if (m_state == AIState::HUNTING) {
        if (result == CellState::HIT) {
            // 首次击中！从搜索模式切换到摧毁模式
            m_state = AIState::TARGETING;
            m_target_hits.push_back(shot);
        }
    }
    else if (m_state == AIState::TARGETING) {
        if (result == CellState::HIT) {
            // 在摧毁模式下再次击中，将新坐标加入目标列表
            m_target_hits.push_back(shot);
        }
        else if (result == CellState::SUNK) {
            // 目标被击沉！返回搜索模式，清空目标列表
            reset();
        }
    }
}