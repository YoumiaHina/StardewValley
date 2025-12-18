#pragma once

#include "cocos2d.h"
#include "Game/GameConfig.h"
#include "Game/PlaceableItemBase.h"
#include "Game/Item.h"

namespace Game {

struct Furnace : public PlaceableItemBase {
    cocos2d::Vec2 dropOffset;
    ItemType oreType = ItemType::CopperGrain;
    float remainingSeconds = 0.0f;

    cocos2d::Rect placeRect() const override;
    cocos2d::Rect collisionRect() const override;
    int maxPerArea() const override { return 200; }
};

struct FurnaceRecipe {
    ItemType ore;
    ItemType fuel;
    ItemType output;
    int oreCount;
    int fuelCount;
    float seconds;
};

inline FurnaceRecipe furnaceRecipeFor(ItemType ore) {
    switch (ore) {
        case ItemType::CopperGrain:
            return FurnaceRecipe{ ItemType::CopperGrain, ItemType::Coal, ItemType::CopperIngot, 5, 1, 30.0f };
        case ItemType::IronGrain:
            return FurnaceRecipe{ ItemType::IronGrain, ItemType::Coal, ItemType::IronIngot, 5, 1, 30.0f };
        case ItemType::GoldGrain:
            return FurnaceRecipe{ ItemType::GoldGrain, ItemType::Coal, ItemType::GoldIngot, 5, 1, 30.0f };
        default:
            return FurnaceRecipe{ ItemType::Wood, ItemType::Wood, ItemType::Wood, 0, 0, 0.0f };
    }
}

inline bool isValidFurnaceRecipe(const FurnaceRecipe& r) {
    return r.oreCount > 0 && r.fuelCount > 0 && r.seconds > 0.0f;
}

inline cocos2d::Rect Furnace::placeRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    cocos2d::Vec2 center = pos;
    float w = s;
    float h = s * 2.0f;
    float minX = center.x - w * 0.5f;
    float minY = center.y - h * 0.5f;
    return cocos2d::Rect(minX, minY, w, h);
}

inline cocos2d::Rect Furnace::collisionRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    cocos2d::Vec2 center = pos;
    float w = s;
    float h = s * 2.0f;
    float minX = center.x - w * 0.5f;
    float midY = center.y;
    return cocos2d::Rect(minX, midY, w, h * 0.5f);
}

} // namespace Game
