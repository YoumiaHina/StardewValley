#pragma once

#include "cocos2d.h"

namespace Game {

// 基础物品类型
enum class ItemType {
    Wood = 0,
    Stone = 1,
    Fiber = 2,
    Chest = 3,
    Parsnip = 4,
    ParsnipSeed = 5,
    Blueberry = 6,
    BlueberrySeed = 7,
    Eggplant = 8,
    EggplantSeed = 9,
    Fish = 100
};

// 英文名称（内部使用）
inline const char* itemName(ItemType t) {
    switch (t) {
        case ItemType::Wood:  return "Wood";
        case ItemType::Stone: return "Stone";
        case ItemType::Fiber: return "Fiber";
        case ItemType::Chest: return "Chest";
        case ItemType::Parsnip: return "Parsnip";
        case ItemType::ParsnipSeed: return "Parsnip Seed";
        case ItemType::Blueberry: return "Blueberry";
        case ItemType::BlueberrySeed: return "Blueberry Seed";
        case ItemType::Eggplant: return "Eggplant";
        case ItemType::EggplantSeed: return "Eggplant Seed";
        case ItemType::Fish: return "Fish";
        default: return "Unknown";
    }
}

// 中文名称（UI显示）
inline const char* itemNameZH(ItemType t) {
    switch (t) {
        case ItemType::Wood:  return "木材";
        case ItemType::Stone: return "石头";
        case ItemType::Fiber: return "纤维";
        case ItemType::Chest: return "箱子";
        case ItemType::Parsnip: return "防风草";
        case ItemType::ParsnipSeed: return "防风草种子";
        case ItemType::Blueberry: return "蓝莓";
        case ItemType::BlueberrySeed: return "蓝莓种子";
        case ItemType::Eggplant: return "茄子";
        case ItemType::EggplantSeed: return "茄子种子";
        case ItemType::Fish: return "鱼";
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
        case ItemType::Chest: return Color4F(0.85f, 0.65f, 0.20f, 1.0f);
        case ItemType::Parsnip: return Color4F(0.95f, 0.85f, 0.35f, 1.0f);
        case ItemType::ParsnipSeed: return Color4F(0.90f, 0.75f, 0.25f, 1.0f);
        case ItemType::Blueberry: return Color4F(0.35f, 0.45f, 0.95f, 1.0f);
        case ItemType::BlueberrySeed: return Color4F(0.35f, 0.45f, 0.95f, 0.8f);
        case ItemType::Eggplant: return Color4F(0.55f, 0.30f, 0.85f, 1.0f);
        case ItemType::EggplantSeed: return Color4F(0.55f, 0.30f, 0.85f, 0.8f);
        case ItemType::Fish: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        default: return Color4F(1,1,1,1);
    }
}

// 是否可食用（占位规则：纤维可食用，用于演示）
inline bool itemEdible(ItemType t) {
    return t == ItemType::Fiber || t == ItemType::Parsnip || t == ItemType::Blueberry || t == ItemType::Eggplant || t == ItemType::Fish;
}

inline int itemEnergyRestore(ItemType t) {
    switch (t) {
        case ItemType::Fiber: return 5;
        case ItemType::Parsnip: return 20;
        case ItemType::Blueberry: return 12;
        case ItemType::Eggplant: return 18;
        case ItemType::Fish: return 10;
        default: return 0;
    }
}

inline int itemHpRestore(ItemType t) {
    switch (t) {
        case ItemType::Fiber: return 0;
        case ItemType::Parsnip: return 0;
        case ItemType::Blueberry: return 2;
        case ItemType::Eggplant: return 4;
        case ItemType::Fish: return 3;
        default: return 0;
    }
}

// 枚举哈希，便于 unordered_map
struct ItemTypeHash {
    std::size_t operator()(ItemType t) const noexcept {
        return static_cast<std::size_t>(t);
    }
};

} // namespace Game
