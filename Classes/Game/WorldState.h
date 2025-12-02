/**
 * WorldState: 全局持久化的游戏状态（背包、农场地图与掉落等）。
 */
#pragma once

#include <memory>
#include <vector>
#include "Game/Inventory.h"
#include "Game/Tile.h"
#include "Game/Drop.h"
#include "Game/GameConfig.h"

namespace Game {

struct WorldState {
    // 共享背包实例（室内外一致）
    std::shared_ptr<Inventory> inventory;

    // 农场地图（按行主序 r*_cols + c）
    std::vector<TileType> farmTiles;

    // 农场掉落（未拾取的物品）
    std::vector<Drop> farmDrops;

    // 热键选中槽位索引
    int selectedIndex = 0;

    // 时间系统：四季与天数（每季 30 天）
    int seasonIndex = 0;   // 0: Spring, 1: Summer, 2: Fall, 3: Winter
    int dayOfSeason = 1;   // 1..30
    int timeHour = 6;      // 0..23, default morning 06:00
    int timeMinute = 0;    // 0..59
    float timeAccum = 0.0f; // real seconds accumulator for minute advancement

    // 能量系统
    int energy = GameConfig::ENERGY_MAX;
    int maxEnergy = GameConfig::ENERGY_MAX;
};

// 获取全局状态（惰性初始化由调用方保证）
WorldState& globalState();

} // namespace Game