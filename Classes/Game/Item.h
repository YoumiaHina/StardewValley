#pragma once

#include "cocos2d.h"
#include <string>

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
    Corn = 10,
    CornSeed = 11,
    Strawberry = 12,
    StrawberrySeed = 13,
    Fish = 100,
    Coal = 101,
    CopperGrain = 102,
    CopperIngot = 103,
    IronGrain   = 104,
    IronIngot   = 105,
    GoldGrain   = 106,
    GoldIngot   = 107,
    Egg         = 200,
    Milk        = 201,
    Wool        = 202
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
        case ItemType::Corn: return "Corn";
        case ItemType::CornSeed: return "Corn Seed";
        case ItemType::Strawberry: return "Strawberry";
        case ItemType::StrawberrySeed: return "Strawberry Seed";
        case ItemType::Fish: return "Fish";
        case ItemType::Coal: return "Coal";
        case ItemType::CopperGrain: return "Copper Grain";
        case ItemType::CopperIngot: return "Copper Ingot";
        case ItemType::IronGrain:   return "Iron Grain";
        case ItemType::IronIngot:   return "Iron Ingot";
        case ItemType::GoldGrain:   return "Gold Grain";
        case ItemType::GoldIngot:   return "Gold Ingot";
        case ItemType::Egg: return "Egg";
        case ItemType::Milk: return "Milk";
        case ItemType::Wool: return "Wool";
        default: return "Unknown";
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
        case ItemType::Corn: return Color4F(0.95f, 0.90f, 0.25f, 1.0f);
        case ItemType::CornSeed: return Color4F(0.95f, 0.90f, 0.25f, 0.8f);
        case ItemType::Strawberry: return Color4F(0.95f, 0.20f, 0.25f, 1.0f);
        case ItemType::StrawberrySeed: return Color4F(0.95f, 0.20f, 0.25f, 0.8f);
        case ItemType::Fish: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Coal: return Color4F(0.15f, 0.15f, 0.15f, 1.0f);
        case ItemType::CopperGrain: return Color4F(0.85f, 0.45f, 0.25f, 1.0f);
        case ItemType::CopperIngot: return Color4F(0.90f, 0.50f, 0.30f, 1.0f);
        case ItemType::IronGrain:   return Color4F(0.60f, 0.60f, 0.65f, 1.0f);
        case ItemType::IronIngot:   return Color4F(0.70f, 0.70f, 0.75f, 1.0f);
        case ItemType::GoldGrain:   return Color4F(0.95f, 0.80f, 0.20f, 1.0f);
        case ItemType::GoldIngot:   return Color4F(0.98f, 0.85f, 0.25f, 1.0f);
        case ItemType::Egg: return Color4F(0.98f, 0.98f, 0.90f, 1.0f);
        case ItemType::Milk: return Color4F(0.95f, 0.95f, 0.85f, 1.0f);
        case ItemType::Wool: return Color4F(0.90f, 0.90f, 0.95f, 1.0f);
        default: return Color4F(1,1,1,1);
    }
}

inline std::string itemIconPath(ItemType t) {
    switch (t) {
        case ItemType::Wood: return "item/Wood.png";
        case ItemType::Stone: return "item/Stone.png";
        case ItemType::Fiber: return "item/Fiber.png";
        case ItemType::Chest: return "PlaceableItem/Chest.png";
        case ItemType::Coal: return "Mineral/Coal.png";
        case ItemType::CopperGrain: return "Mineral/copperGrain.png";
        case ItemType::CopperIngot: return "Mineral/copperIngot.png";
        case ItemType::IronGrain:   return "Mineral/ironGrain.png";
        case ItemType::IronIngot:   return "Mineral/ironIngot.png";
        case ItemType::GoldGrain:   return "Mineral/goldGrain.png";
        case ItemType::GoldIngot:   return "Mineral/goldIngot.png";
        case ItemType::Parsnip: return "DropsAndInventory/Vegetable/Parsnip.png";
        case ItemType::Blueberry: return "DropsAndInventory/Vegetable/Blueberry.png";
        case ItemType::Eggplant: return "DropsAndInventory/Vegetable/Eggplant.png";
        case ItemType::Corn: return "DropsAndInventory/Vegetable/Corn.png";
        case ItemType::Strawberry: return "DropsAndInventory/Vegetable/Strawberry.png";
        case ItemType::ParsnipSeed: return "DropsAndInventory/Seeds/Parsnip_Seeds.png";
        case ItemType::BlueberrySeed: return "DropsAndInventory/Seeds/Blueberry_Seeds.png";
        case ItemType::EggplantSeed: return "DropsAndInventory/Seeds/Eggplant_Seeds.png";
        case ItemType::CornSeed: return "DropsAndInventory/Seeds/Corn_Seeds.png";
        case ItemType::StrawberrySeed: return "DropsAndInventory/Seeds/Strawberry_Seeds.png";
        case ItemType::Egg: return "DropsAndInventory/Animal_Products/Egg.png";
        case ItemType::Milk: return "DropsAndInventory/Animal_Products/Milk.png";
        case ItemType::Wool: return "DropsAndInventory/Animal_Products/Wool.png";
        default: return std::string();
    }
}

// 是否可食用（占位规则：纤维可食用，用于演示）
inline bool itemEdible(ItemType t) {
    return t == ItemType::Fiber || t == ItemType::Parsnip || t == ItemType::Blueberry || t == ItemType::Eggplant || t == ItemType::Corn || t == ItemType::Strawberry || t == ItemType::Fish || t == ItemType::Egg || t == ItemType::Milk;
}

inline int itemEnergyRestore(ItemType t) {
    switch (t) {
        case ItemType::Fiber: return 5;
        case ItemType::Parsnip: return 25;
        case ItemType::Blueberry: return 25;
        case ItemType::Eggplant: return 20;
        case ItemType::Corn: return 25;
        case ItemType::Strawberry: return 50;
        case ItemType::Fish: return 15;
        case ItemType::Egg: return 15;
        case ItemType::Milk: return 20;
        default: return 0;
    }
}

// 物品售价（成熟作物固有属性；种子商店售价由商店逻辑另行定义，此处仅为物品本身价值）
inline int itemPrice(ItemType t) {
    switch (t) {
        case ItemType::Parsnip: return 35;
        case ItemType::Blueberry: return 50;
        case ItemType::Eggplant: return 60;
        case ItemType::Corn: return 50;
        case ItemType::Strawberry: return 120;
        case ItemType::Fish: return 75;
        case ItemType::Coal: return 20;
        case ItemType::CopperGrain: return 20;
        case ItemType::CopperIngot: return 90;
        case ItemType::IronGrain:   return 25;
        case ItemType::IronIngot:   return 110;
        case ItemType::GoldGrain:   return 40;
        case ItemType::GoldIngot:   return 180;
        case ItemType::Egg: return 50;
        case ItemType::Milk: return 125;
        case ItemType::Wool: return 340;
        // 种子本身也可以有售价，通常低于买入价，或者不可卖出。
        // 这里暂时定义种子售价为 0 或者一半？用户只说了买入25。
        // 假设卖出价格：
        default: return 0;
    }
}

inline int itemHpRestore(ItemType t) {
    switch (t) {
        case ItemType::Fiber: return 0;
        case ItemType::Parsnip: return 0;
        case ItemType::Blueberry: return 2;
        case ItemType::Eggplant: return 4;
        case ItemType::Corn: return 2;
        case ItemType::Strawberry: return 2;
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
