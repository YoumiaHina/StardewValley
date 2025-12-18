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

FurnaceRecipe furnaceRecipeFor(ItemType ore);
bool isValidFurnaceRecipe(const FurnaceRecipe& r);

} // namespace Game
