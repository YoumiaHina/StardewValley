#pragma once

#include "cocos2d.h"

namespace Game {

// 基础物品类型
enum class ItemType {
    Wood = 0,
    Stone = 1,
    Fiber = 2,
};

// 英文名称（内部使用）
inline const char* itemName(ItemType t) {
    switch (t) {
        case ItemType::Wood:  return "Wood";
        case ItemType::Stone: return "Stone";
        case ItemType::Fiber: return "Fiber";
        default: return "Unknown";
    }
}

// 中文名称（UI显示）
inline const char* itemNameZH(ItemType t) {
    switch (t) {
        case ItemType::Wood:  return "木材";
        case ItemType::Stone: return "石头";
        case ItemType::Fiber: return "纤维";
        default: return "未知";
    }
}

// 物品代表颜色（用于掉落渲染）
inline cocos2d::Color4F itemColor(ItemType t) {
    using cocos2d::Color4F;
    switch (t) {
        case ItemType::Wood:  return Color4F(0.55f, 0.40f, 0.25f, 1.0f);
        case ItemType::Stone: return Color4F(0.65f, 0.65f, 0.68f, 1.0f);
        case ItemType::Fiber: return Color4F(0.25f, 0.75f, 0.35f, 1.0f);
        default: return Color4F(1,1,1,1);
    }
}

// 枚举哈希，便于 unordered_map
struct ItemTypeHash {
    std::size_t operator()(ItemType t) const noexcept {
        return static_cast<std::size_t>(t);
    }
};

} // namespace Game