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
    Carp = 101,
    BreamFish = 102,
    Sardine = 103,
    Salmon = 104,
    RainbowTrout = 105,
    MidnightCarp = 106,
    LargemouthBass = 107,
    Sturgeon = 108,
    SmallmouthBass = 109,
    Tilapia = 110,
    Tuna = 111,
    Globefish = 112,
    Anchovy = 113,
    BlueDiscus = 114,
    Clam = 115,
    Crab = 116,
    Lobster = 117,
    Shrimp = 118,
    Coal = 120,
    CopperGrain = 121,
    CopperIngot = 122,
    IronGrain   = 123,
    IronIngot   = 124,
    GoldGrain   = 125,
    GoldIngot   = 126,
    Egg         = 200,
    Milk        = 201,
    Wool        = 202,
    Furnace     = 300
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
        case ItemType::Carp: return "Carp";
        case ItemType::BreamFish: return "Bream";
        case ItemType::Sardine: return "Sardine";
        case ItemType::Salmon: return "Salmon";
        case ItemType::RainbowTrout: return "Rainbow Trout";
        case ItemType::MidnightCarp: return "Midnight Carp";
        case ItemType::LargemouthBass: return "Largemouth Bass";
        case ItemType::Sturgeon: return "Sturgeon";
        case ItemType::SmallmouthBass: return "Smallmouth Bass";
        case ItemType::Tilapia: return "Tilapia";
        case ItemType::Tuna: return "Tuna";
        case ItemType::Globefish: return "Globefish";
        case ItemType::Anchovy: return "Anchovy";
        case ItemType::BlueDiscus: return "Blue Discus";
        case ItemType::Clam: return "Clam";
        case ItemType::Crab: return "Crab";
        case ItemType::Lobster: return "Lobster";
        case ItemType::Shrimp: return "Shrimp";
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
        case ItemType::Furnace: return "Furnace";
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
        case ItemType::Carp: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::BreamFish: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Sardine: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Salmon: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::RainbowTrout: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::MidnightCarp: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::LargemouthBass: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Sturgeon: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::SmallmouthBass: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Tilapia: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Tuna: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Globefish: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Anchovy: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::BlueDiscus: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Clam: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Crab: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Lobster: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
        case ItemType::Shrimp: return Color4F(0.25f, 0.55f, 1.0f, 1.0f);
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
        case ItemType::Furnace: return Color4F(0.80f, 0.50f, 0.20f, 1.0f);
        default: return Color4F(1,1,1,1);
    }
}

inline std::string itemIconPath(ItemType t) {
    switch (t) {
        case ItemType::Wood: return "item/Wood.png";
        case ItemType::Stone: return "item/Stone.png";
        case ItemType::Fiber: return "item/Fiber.png";
        case ItemType::Chest: return "PlaceableItem/Chest.png";
        case ItemType::Furnace: return "PlaceableItem/Furnace.png";
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
        case ItemType::Fish: return "fish/Bream.png";
        case ItemType::Carp: return "fish/carp.png";
        case ItemType::BreamFish: return "fish/Bream.png";
        case ItemType::Sardine: return "fish/Sardine.png";
        case ItemType::Salmon: return "fish/Salmon.png";
        case ItemType::RainbowTrout: return "fish/Rainbow_Trout.png";
        case ItemType::MidnightCarp: return "fish/Midnight_Carp.png";
        case ItemType::LargemouthBass: return "fish/Largemouth_Bass.png";
        case ItemType::Sturgeon: return "fish/Sturgeon.png";
        case ItemType::SmallmouthBass: return "fish/Smallmouth_Bass.png";
        case ItemType::Tilapia: return "fish/Tilapia.png";
        case ItemType::Tuna: return "fish/Tuna.png";
        case ItemType::Globefish: return "fish/globefish.png";
        case ItemType::Anchovy: return "fish/Anchovy.png";
        case ItemType::BlueDiscus: return "fish/Blue_Discus.png";
        case ItemType::Clam: return "fish/Clam.png";
        case ItemType::Crab: return "fish/Crab.png";
        case ItemType::Lobster: return "fish/Lobster.png";
        case ItemType::Shrimp: return "fish/Shrimp.png";
        default: return std::string();
    }
}

inline bool itemEdible(ItemType t) {
    return t == ItemType::Fiber
        || t == ItemType::Parsnip
        || t == ItemType::Blueberry
        || t == ItemType::Eggplant
        || t == ItemType::Corn
        || t == ItemType::Strawberry
        || t == ItemType::Fish
        || t == ItemType::Carp
        || t == ItemType::BreamFish
        || t == ItemType::Sardine
        || t == ItemType::Salmon
        || t == ItemType::RainbowTrout
        || t == ItemType::MidnightCarp
        || t == ItemType::LargemouthBass
        || t == ItemType::Sturgeon
        || t == ItemType::SmallmouthBass
        || t == ItemType::Tilapia
        || t == ItemType::Tuna
        || t == ItemType::Globefish
        || t == ItemType::Anchovy
        || t == ItemType::BlueDiscus
        || t == ItemType::Clam
        || t == ItemType::Crab
        || t == ItemType::Lobster
        || t == ItemType::Shrimp
        || t == ItemType::Egg
        || t == ItemType::Milk;
}

inline bool isFish(ItemType t) {
    switch (t) {
        case ItemType::Fish:
        case ItemType::Carp:
        case ItemType::BreamFish:
        case ItemType::Sardine:
        case ItemType::Salmon:
        case ItemType::RainbowTrout:
        case ItemType::MidnightCarp:
        case ItemType::LargemouthBass:
        case ItemType::Sturgeon:
        case ItemType::SmallmouthBass:
        case ItemType::Tilapia:
        case ItemType::Tuna:
        case ItemType::Globefish:
        case ItemType::Anchovy:
        case ItemType::BlueDiscus:
        case ItemType::Clam:
        case ItemType::Crab:
        case ItemType::Lobster:
        case ItemType::Shrimp:
            return true;
        default:
            return false;
    }
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
        case ItemType::Carp: return 15;
        case ItemType::BreamFish: return 15;
        case ItemType::Sardine: return 15;
        case ItemType::Salmon: return 20;
        case ItemType::RainbowTrout: return 18;
        case ItemType::MidnightCarp: return 22;
        case ItemType::LargemouthBass: return 18;
        case ItemType::Sturgeon: return 25;
        case ItemType::SmallmouthBass: return 15;
        case ItemType::Tilapia: return 16;
        case ItemType::Tuna: return 24;
        case ItemType::Globefish: return 15;
        case ItemType::Anchovy: return 12;
        case ItemType::BlueDiscus: return 22;
        case ItemType::Clam: return 10;
        case ItemType::Crab: return 18;
        case ItemType::Lobster: return 20;
        case ItemType::Shrimp: return 14;
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
        case ItemType::Carp: return 400;
        case ItemType::BreamFish: return 450;
        case ItemType::Sardine: return 300;
        case ItemType::Salmon: return 900;
        case ItemType::RainbowTrout: return 800;
        case ItemType::MidnightCarp: return 1100;
        case ItemType::LargemouthBass: return 700;
        case ItemType::Sturgeon: return 1800;
        case ItemType::SmallmouthBass: return 500;
        case ItemType::Tilapia: return 550;
        case ItemType::Tuna: return 1300;
        case ItemType::Globefish: return 750;
        case ItemType::Anchovy: return 350;
        case ItemType::BlueDiscus: return 1400;
        case ItemType::Clam: return 300;
        case ItemType::Crab: return 1000;
        case ItemType::Lobster: return 1200;
        case ItemType::Shrimp: return 600;
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
        case ItemType::Carp: return 3;
        case ItemType::BreamFish: return 3;
        case ItemType::Sardine: return 2;
        case ItemType::Salmon: return 4;
        case ItemType::RainbowTrout: return 3;
        case ItemType::MidnightCarp: return 4;
        case ItemType::LargemouthBass: return 3;
        case ItemType::Sturgeon: return 5;
        case ItemType::SmallmouthBass: return 3;
        case ItemType::Tilapia: return 3;
        case ItemType::Tuna: return 4;
        case ItemType::Globefish: return 3;
        case ItemType::Anchovy: return 2;
        case ItemType::BlueDiscus: return 4;
        case ItemType::Clam: return 1;
        case ItemType::Crab: return 3;
        case ItemType::Lobster: return 4;
        case ItemType::Shrimp: return 2;
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
