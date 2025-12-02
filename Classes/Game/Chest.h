#pragma once

#include "cocos2d.h"
#include "Game/Bag.h"

namespace Game {

// 可放置的箱子：位置 + 内部物品（简单计数包）
struct Chest {
    cocos2d::Vec2 pos; // 世界坐标（格子中心）
    Bag bag;           // 箱子内部存储（按物品类型计数）
};

} // namespace Game