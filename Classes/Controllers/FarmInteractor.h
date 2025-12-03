/**
 * FarmInteractor: 处理农场场景中的空格键交互（进屋、播种、放置/存入箱子）。
 */
#pragma once

#include <memory>
#include <string>
#include "cocos2d.h"
#include "Game/Inventory.h"
#include "Controllers/IMapController.h"
#include "Controllers/UIController.h"
#include <functional>

namespace Controllers {

class FarmInteractor {
public:
    enum class SpaceAction { None, EnterHouse };

    FarmInteractor(std::shared_ptr<Game::Inventory> inventory,
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