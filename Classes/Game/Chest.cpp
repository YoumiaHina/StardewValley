#include "Game/Chest.h"

using namespace cocos2d;

namespace Game {

Rect Chest::placeRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return PlaceableItemBase::standard1x2PlaceRect(pos, s);
}

Rect Chest::collisionRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return PlaceableItemBase::standardBottomHalfCollisionRect(pos, s);
}

Rect chestRect(const Chest& chest) {
    return chest.placeRect();
}

Rect chestCollisionRect(const Chest& chest) {
    return chest.collisionRect();
}

bool isNearAnyChest(const Vec2& playerWorldPos, const std::vector<Chest>& chests) {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float margin = s * 0.4f;
    return PlaceableItemBase::isNearAny<Chest>(
        playerWorldPos,
        chests,
        [](const Chest& c) { return chestRect(c); },
        margin);
}

} // namespace Game

