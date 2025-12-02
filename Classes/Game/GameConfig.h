/**
 * GameConfig: 全局游戏常量与配置占位
 */
#pragma once

namespace GameConfig {
    // 设计分辨率（占位，保持与现有AppDelegate默认一致以避免UI改变）
    static const int DESIGN_WIDTH  = 480;
    static const int DESIGN_HEIGHT = 320;

    // 基础地图/格子设定（后续可用于TMX或自研地图）
    static const int TILE_SIZE     = 16;

    // 过渡时间
    static const float TRANSITION_FADE = 0.3f;

    // 工具栏槽位数量（模仿星露谷初始背包，上排 12 格）
    static const int TOOLBAR_SLOTS = 12;

    // 地图网格尺寸：扩容，提供更大的农场可探索范围
    static const int MAP_COLS = 120;
    static const int MAP_ROWS = 80;

    // 掉落与拾取相关参数
    static const float DROP_DRAW_RADIUS = 8.0f;   // 掉落渲染圆点半径
    static const float DROP_PICK_RADIUS = 20.0f;  // 玩家拾取距离阈值

    // 能量系统（基础与消耗/回复）
    static const int ENERGY_MAX = 100;
    static const int ENERGY_COST_AXE = 8;
    static const int ENERGY_COST_PICKAXE = 8;
    static const int ENERGY_COST_HOE = 4;
    static const int ENERGY_COST_WATER = 2;
    static const int ENERGY_RECOVER_FIBER = 15; // 纤维作为占位食物

    // Time mapping
    // Seconds per one in-game minute: 10s -> +10min implies 1s -> +1min
    static const float REAL_SECONDS_PER_GAME_MINUTE = 1.0f;
    // Kept for reference if hour-based mapping is needed elsewhere
    static const float REAL_SECONDS_PER_GAME_HOUR = 60.0f;
}