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

    // 工具栏槽位数量
    static const int TOOLBAR_SLOTS = 4;

    // 地图网格尺寸（按 480x320 与 16 像素格计算）
    static const int MAP_COLS = 30; // 480 / 16
    static const int MAP_ROWS = 20; // 320 / 16
}