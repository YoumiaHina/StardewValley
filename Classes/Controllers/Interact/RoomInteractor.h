/**
 * RoomInteractor: 处理室内场景的空格键交互（出屋、睡觉）。
 */
#pragma once

#include <memory>
#include "cocos2d.h"
#include "Game/Inventory.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/Interact/ChestInteractor.h"
#include <functional>

namespace Controllers {

class GameStateController;

class RoomInteractor {
public:
    enum class SpaceAction { None, ExitHouse, Slept };

    RoomInteractor(std::shared_ptr<Game::Inventory> inventory,
                   Controllers::IMapController* map,
                   Controllers::UIController* ui,
                   Controllers::CropSystem* crop,
                   Controllers::GameStateController* state,
                   std::function<cocos2d::Vec2()> getPlayerPos)
    : _inventory(std::move(inventory)), _map(map), _ui(ui), _crop(crop), _state(state), _getPlayerPos(std::move(getPlayerPos)) {}

    SpaceAction onSpacePressed();
    void onLeftClick();

private:
    std::shared_ptr<Game::Inventory> _inventory;
    Controllers::IMapController* _map = nullptr;
    Controllers::UIController* _ui = nullptr;
    Controllers::CropSystem* _crop = nullptr;
    Controllers::GameStateController* _state = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
    Controllers::ChestInteractor* _chestInteractor = nullptr;
};

}
// namespace Controllers
