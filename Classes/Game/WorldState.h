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
};

// 获取全局状态（惰性初始化由调用方保证）
WorldState& globalState();

} // namespace Game