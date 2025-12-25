// 室内交互器：
// - 作用：封装室内场景的“空格交互/点击交互”判定入口（出门、睡觉、与室内可交互物体交互）。
// - 职责边界：只做交互判定与结果返回/转发，不直接驱动场景切换与日结算逻辑。
// - 主要协作对象：IMapController 提供地图查询；UIController 提供提示/面板；GameStateController 负责推进天数等全局状态；作物系统用于睡觉后的每日推进链路入口。
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
class FurnaceController;

// 室内交互器：根据玩家位置判定空格/点击应触发的交互动作。
class RoomInteractor {
public:
    // 空格键在室内可能触发的动作枚举；调用方根据返回值做后续处理。
    enum class SpaceAction { None, ExitHouse, Slept };

    // 构造：注入背包、地图、UI、作物系统、游戏状态控制器，以及获取玩家位置的回调。
    RoomInteractor(std::shared_ptr<Game::Inventory> inventory,
                   Controllers::IMapController* map,
                   Controllers::UIController* ui,
                   Controllers::CropSystem* crop,
                   Controllers::GameStateController* state,
                   std::function<cocos2d::Vec2()> getPlayerPos)
    : _inventory(std::move(inventory)), _map(map), _ui(ui), _crop(crop), _state(state), _getPlayerPos(std::move(getPlayerPos)) {}

    // 处理空格键交互：返回应执行的场景动作（出门/睡觉等）。
    SpaceAction onSpacePressed();
    // 处理鼠标左键点击：用于箱子等室内交互入口转发。
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
