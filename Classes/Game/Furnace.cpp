#include "Game/Furnace.h"

using namespace cocos2d;

namespace Game {

Rect Furnace::placeRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return PlaceableItemBase::standard1x2PlaceRect(pos, s);
}

Rect Furnace::collisionRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return PlaceableItemBase::standardBottomHalfCollisionRect(pos, s);
}

FurnaceRecipe furnaceRecipeFor(ItemType ore) {
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

bool isValidFurnaceRecipe(const FurnaceRecipe& r) {
    return r.oreCount > 0 && r.fuelCount > 0 && r.seconds > 0.0f;
}

} // namespace Game
