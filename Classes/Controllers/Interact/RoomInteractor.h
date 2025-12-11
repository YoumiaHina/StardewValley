/**
 * RoomInteractor: 处理室内场景的空格键交互（出屋、睡觉、箱子操作）。
 */
#pragma once

#include <memory>
#include "cocos2d.h"
#include "Game/Inventory.h"
#include "Controllers/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include <functional>

namespace Controllers {

class RoomInteractor {
public:
    enum class SpaceAction { None, ExitHouse, Slept };

    RoomInteractor(std::shared_ptr<Game::Inventory> inventory,
                   Controllers::IMapController* map,
                   Controllers::UIController* ui,
                   Controllers::CropSystem* crop,
                   std::function<cocos2d::Vec2()> getPlayerPos)
    : _inventory(std::move(inventory)), _map(map), _ui(ui), _crop(crop), _getPlayerPos(std::move(getPlayerPos)) {}

    SpaceAction onSpacePressed();

private:
    std::shared_ptr<Game::Inventory> _inventory;
    Controllers::IMapController* _map = nullptr;
    Controllers::UIController* _ui = nullptr;
    Controllers::CropSystem* _crop = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
};

}
// namespace Controllers
