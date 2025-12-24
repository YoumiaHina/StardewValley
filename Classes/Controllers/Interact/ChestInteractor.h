#pragma once

#include <memory>
#include <functional>
#include "cocos2d.h"
#include "Game/Inventory.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"

namespace Controllers {

// 箱子交互器：
// - 作用：封装“点击箱子/打开箱子面板”的交互入口。
// - 职责边界：只处理输入触发与目标判定/转发，不维护箱子系统状态。
// - 主要协作对象：IMapController 提供地图与箱子查询；UIController 负责面板展示；玩家位置/方向由回调提供。
class ChestInteractor {
public:
    // 构造：注入背包、地图与 UI 依赖，以及获取玩家位置/朝向的回调。
    ChestInteractor(std::shared_ptr<Game::Inventory> inventory,
                    Controllers::IMapController* map,
                    Controllers::UIController* ui,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir)
    : _inventory(std::move(inventory)), _map(map), _ui(ui),
      _getPlayerPos(std::move(getPlayerPos)), _getLastDir(std::move(getLastDir)) {}

    // 处理鼠标左键点击：尝试与玩家面前/附近的箱子交互并打开面板。
    void onLeftClick();

private:
    std::shared_ptr<Game::Inventory> _inventory;
    Controllers::IMapController* _map = nullptr;
    Controllers::UIController* _ui = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
    std::function<cocos2d::Vec2()> _getLastDir;
};

}
