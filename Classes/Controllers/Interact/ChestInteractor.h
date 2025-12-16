#pragma once

#include <memory>
#include <functional>
#include "cocos2d.h"
#include "Game/Inventory.h"
#include "Controllers/IMapController.h"
#include "Controllers/UI/UIController.h"

namespace Controllers {

class ChestInteractor {
public:
    ChestInteractor(std::shared_ptr<Game::Inventory> inventory,
                    Controllers::IMapController* map,
                    Controllers::UIController* ui,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir)
    : _inventory(std::move(inventory)), _map(map), _ui(ui),
      _getPlayerPos(std::move(getPlayerPos)), _getLastDir(std::move(getLastDir)) {}

    void onLeftClick();

private:
    std::shared_ptr<Game::Inventory> _inventory;
    Controllers::IMapController* _map = nullptr;
    Controllers::UIController* _ui = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
    std::function<cocos2d::Vec2()> _getLastDir;
};

}
