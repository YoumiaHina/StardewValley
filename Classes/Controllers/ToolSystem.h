/**
 * ToolSystem: 工具使用逻辑（锄地、砍树、采矿、浇水、收获）。
 */
#pragma once

#include <string>
#include "cocos2d.h"
#include "Game/Inventory.h"
#include "Game/Tool.h"
#include "Game/WorldState.h"
#include "Controllers/IMapController.h"
#include "Controllers/CropSystem.h"
#include "Controllers/UIController.h"
#include <functional>

namespace Controllers {

class ToolSystem {
public:
    ToolSystem(std::shared_ptr<Game::Inventory> inventory,
               IMapController* map,
               CropSystem* crop,
               std::function<cocos2d::Vec2()> getPlayerPos,
               std::function<cocos2d::Vec2()> getLastDir,
               UIController* ui)
    : _inventory(std::move(inventory)), _map(map), _crop(crop), _getPlayerPos(std::move(getPlayerPos)), _getLastDir(std::move(getLastDir)), _ui(ui) {}

    // 使用当前选中工具；返回提示消息。
    std::string useSelectedTool();

private:
    std::shared_ptr<Game::Inventory> _inventory;
    IMapController* _map = nullptr;
    CropSystem* _crop = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos; // 提示定位回调
    std::function<cocos2d::Vec2()> _getLastDir;   // 面向方向回调
    UIController* _ui = nullptr;
};

} // namespace Controllers
