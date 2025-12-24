#pragma once

#include "Controllers/UI/UIController.h"
#include "Controllers/Map/BeachMapController.h"
#include "Game/Inventory.h"

namespace Controllers {

// 沙滩交互器：
// - 作用：封装沙滩场景的“空格交互”判定入口，并返回应执行的场景动作枚举。
// - 职责边界：只做交互判定与结果返回，不负责场景切换与 UI 实现细节。
// - 主要协作对象：BeachMapController 提供地图查询；UIController 提供提示/面板能力；玩家位置通过回调获取。
class BeachInteractor {
public:
    // 空格键在沙滩中可能触发的动作枚举；调用方根据返回值做后续处理。
    enum class SpaceAction { None, EnterFarm };

    // 设置沙滩地图控制器（仅用于接口调用，不负责释放）。
    void setMap(BeachMapController* m) { _map = m; }
    // 设置背包指针（用于后续交互读取/写入物品，不负责释放）。
    void setInventory(Game::Inventory* inv) { _inventory = inv; }
    // 设置 UI 控制器（用于提示/面板展示，不负责释放）。
    void setUI(UIController* ui) { _ui = ui; }
    // 设置获取玩家世界坐标的回调（用于按键时拿到最新位置）。
    void setGetPlayerPos(std::function<cocos2d::Vec2()> f) { _getPlayerPos = std::move(f); }

    // 处理空格键交互：根据玩家位置与地图信息返回应执行的动作。
    SpaceAction onSpacePressed();

private:
    BeachMapController* _map = nullptr;
    Game::Inventory* _inventory = nullptr;
    UIController* _ui = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
};

} // namespace Controllers

