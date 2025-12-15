#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "Game/GameConfig.h"
#include "Game/Inventory.h"

namespace Controllers {
class IMapController;
class UIController;
class RoomMapController;
}

namespace Game {

// 可放置的箱子：位置 + 内部物品槽（模拟三栏物品栏）
struct Chest {
    static constexpr int ROWS = 3;
    static constexpr int COLS = 12;
    static constexpr int CAPACITY = ROWS * COLS;

    cocos2d::Vec2 pos;
    std::vector<Slot> slots;

    Chest()
      : pos(cocos2d::Vec2::ZERO),
        slots(static_cast<std::size_t>(CAPACITY)) {}
};

cocos2d::Rect chestRect(const Chest& chest);
cocos2d::Rect chestCollisionRect(const Chest& chest);
bool isNearAnyChest(const cocos2d::Vec2& playerWorldPos, const std::vector<Chest>& chests);

bool openChestNearPlayer(Controllers::IMapController* map,
                         Controllers::UIController* ui,
                         const cocos2d::Vec2& playerWorldPos,
                         const cocos2d::Vec2& lastDir);

bool placeChestOnFarm(Controllers::IMapController* map,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const cocos2d::Vec2& playerPos,
                      const cocos2d::Vec2& lastDir);

bool placeChestInRoom(Controllers::RoomMapController* room,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const cocos2d::Vec2& playerPos);

void transferChestCell(Game::Chest& chest,
                       int flatIndex,
                       Game::Inventory& inventory);

} // namespace Game
