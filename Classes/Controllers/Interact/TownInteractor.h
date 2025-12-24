#pragma once

#include "cocos2d.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/NPC/NpcControllerBase.h"
#include <functional>

namespace Controllers {

// 城镇交互器：
// - 作用：封装城镇场景的“空格交互”判定入口，并返回应执行的场景动作枚举。
// - 职责边界：只做交互判定与结果返回，不负责场景切换与 UI 实现细节。
// - 主要协作对象：TownMapController 提供地图查询；NpcControllerBase 提供对话/交互入口；UIController 负责提示/面板；玩家位置通过回调获取。
class TownInteractor {
public:
    // 空格键在城镇中可能触发的动作枚举；调用方根据返回值做后续处理。
    enum class SpaceAction { None, EnterFarm };

    // 设置城镇地图控制器（仅用于接口调用，不负责释放）。
    void setMap(TownMapController* m) { _map = m; }
    // 设置获取玩家世界坐标的回调（用于按键时拿到最新位置）。
    void setGetPlayerPos(std::function<cocos2d::Vec2()> f) { _getPlayerPos = std::move(f); }
    // 设置 NPC 控制器（用于对话等交互，不负责释放）。
    void setNpcController(NpcControllerBase* c) { _npc = c; }
    // 设置 UI 控制器（用于提示/面板展示，不负责释放）。
    void setUI(Controllers::UIController* ui) { _ui = ui; }
    // 设置背包（用于交互可能需要的物品读取/写入）。
    void setInventory(std::shared_ptr<Game::Inventory> inv) { _inventory = std::move(inv); }

    // 处理空格键交互：根据玩家位置与地图信息返回应执行的动作。
    SpaceAction onSpacePressed();

private:
    Controllers::TownMapController* _map = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
    Controllers::NpcControllerBase* _npc = nullptr;
    Controllers::UIController* _ui = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
};

} // namespace Controllers
