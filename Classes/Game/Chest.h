#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "Game/GameConfig.h"
#include "Game/Inventory.h"
#include "Game/PlaceableItemBase.h"

namespace Controllers {
class IMapController;
class UIController;
class RoomMapController;
}

namespace Game {

struct Chest : public PlaceableItemBase {
    static constexpr int ROWS = 3;
    static constexpr int COLS = 12;
    static constexpr int CAPACITY = ROWS * COLS;
    static constexpr int MAX_PER_AREA = 200;

    std::vector<Slot> slots;

    Chest()
      : PlaceableItemBase(),
        slots(static_cast<std::size_t>(CAPACITY)) {}

    cocos2d::Rect placeRect() const override;
    cocos2d::Rect collisionRect() const override;
    int maxPerArea() const override { return MAX_PER_AREA; }
};

cocos2d::Rect chestRect(const Chest& chest);
cocos2d::Rect chestCollisionRect(const Chest& chest);
bool isNearAnyChest(const cocos2d::Vec2& playerWorldPos, const std::vector<Chest>& chests);

} // namespace Game
