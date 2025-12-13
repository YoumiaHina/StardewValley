/**
 * FarmInteractor: 处理农场场景中的空格键交互（进屋、播种、放置/存入箱子）。
 */
#pragma once

#include <memory>
#include <string>
#include "cocos2d.h"
#include "Game/Inventory.h"
#include "Controllers/IMapController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/UI/UIController.h"
#include <functional>

namespace Controllers {

class FarmInteractor {
public:
    enum class SpaceAction { None, EnterHouse, EnterMine, EnterBeach };

    FarmInteractor(std::shared_ptr<Game::Inventory> inventory,
                   Controllers::IMapController* map,
                   Controllers::UIController* ui,
                   Controllers::CropSystem* crop,
                   std::function<cocos2d::Vec2()> getPlayerPos,
                   std::function<cocos2d::Vec2()> getLastDir)
    : _inventory(std::move(inventory)), _map(map), _ui(ui), _crop(crop), _getPlayerPos(std::move(getPlayerPos)), _getLastDir(std::move(getLastDir)) {}

    SpaceAction onSpacePressed();

private:
    std::shared_ptr<Game::Inventory> _inventory;
    Controllers::IMapController* _map = nullptr;
    Controllers::UIController* _ui = nullptr;
    Controllers::CropSystem* _crop = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
    std::function<cocos2d::Vec2()> _getLastDir;
};

}
// namespace Controllers
