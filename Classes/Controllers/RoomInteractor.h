/**
 * RoomInteractor: 处理室内场景的空格键交互（出屋、睡觉、箱子操作）。
 */
#pragma once

#include <memory>
#include "cocos2d.h"
#include "Game/Inventory.h"
#include "Controllers/IMapController.h"
#include "Controllers/UIController.h"
#include <functional>

namespace Controllers {

class RoomInteractor {
public:
    enum class SpaceAction { None, ExitHouse, Slept };

    RoomInteractor(std::shared_ptr<Game::Inventory> inventory,
                   IMapController* map,
                   UIController* ui,
                   std::function<cocos2d::Vec2()> getPlayerPos)
    : _inventory(std::move(inventory)), _map(map), _ui(ui), _getPlayerPos(std::move(getPlayerPos)) {}

    SpaceAction onSpacePressed();

private:
    std::shared_ptr<Game::Inventory> _inventory;
    IMapController* _map = nullptr;
    UIController* _ui = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
};

} // namespace Controllers