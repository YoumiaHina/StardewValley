// 农场交互器：
// - 作用：封装农场场景的空格交互与点击交互入口（进门、与 NPC/箱子交互、种植/浇水等入口转发）。
// - 职责边界：只做交互判定与结果返回/转发，不负责场景切换与具体业务规则实现。
// - 主要协作对象：IMapController 提供地图/瓦片查询；CropSystem/AnimalSystem 提供系统操作接口；UIController 负责提示/面板；NPC 控制器提供对话入口。
#pragma once

#include <memory>
#include <string>
#include "cocos2d.h"
#include "Game/Inventory.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/AnimalSystem.h"
#include "Controllers/Interact/ChestInteractor.h"
#include <functional>

namespace Controllers {

class NpcControllerBase;

// 农场交互器：根据玩家位置与朝向判定空格/点击应触发的交互动作。
class FarmInteractor {
public:
    // 空格键在农场中可能触发的动作枚举；调用方根据返回值做后续处理。
    enum class SpaceAction { None, EnterHouse, EnterMine, EnterBeach, EnterTown };

    // 构造：注入背包、地图、UI、作物/动物系统，以及获取玩家位置/朝向的回调。
    FarmInteractor(std::shared_ptr<Game::Inventory> inventory,
                   Controllers::IMapController* map,
                   Controllers::UIController* ui,
                   Controllers::CropSystem* crop,
                   Controllers::AnimalSystem* animals,
                   std::function<cocos2d::Vec2()> getPlayerPos,
                   std::function<cocos2d::Vec2()> getLastDir)
    : _inventory(std::move(inventory)), _map(map), _ui(ui), _crop(crop), _animals(animals), _getPlayerPos(std::move(getPlayerPos)), _getLastDir(std::move(getLastDir)) {}

    // 设置 NPC 控制器（用于对话等交互，不负责释放）。
    void setNpcController(NpcControllerBase* npc) { _npc = npc; }

    // 处理空格键交互：返回应执行的场景动作（进门/切场景等）。
    SpaceAction onSpacePressed();
    // 处理鼠标左键点击：用于箱子/NPC 等交互入口转发。
    void onLeftClick();

private:
    std::shared_ptr<Game::Inventory> _inventory;
    Controllers::IMapController* _map = nullptr;
    Controllers::UIController* _ui = nullptr;
    Controllers::CropSystem* _crop = nullptr;
    Controllers::AnimalSystem* _animals = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
    std::function<cocos2d::Vec2()> _getLastDir;
    Controllers::ChestInteractor* _chestInteractor = nullptr;
    NpcControllerBase* _npc = nullptr;
};

}
// namespace Controllers
