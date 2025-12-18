#include "Game/Chest.h"

using namespace cocos2d;

namespace Game {

Rect Chest::placeRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    Vec2 center = pos;
    float w = s;
    float h = s * 2.0f;
    float minX = center.x - w * 0.5f;
    float minY = center.y - h * 0.5f;
    return Rect(minX, minY, w, h);
}

Rect Chest::collisionRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    Vec2 center = pos;
    float w = s;
    float h = s * 2.0f;
    float minX = center.x - w * 0.5f;
    float midY = center.y;
    return Rect(minX, midY, w, h * 0.5f);
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

