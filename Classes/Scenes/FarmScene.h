/**
 * FarmScene：农场场景（继承 SceneBase），负责农场地图的场景骨架与事件转发。
 * - 职责边界：只做场景初始化/出生点选择/输入事件转发/场景切换，不承载农场业务规则。
 * - 主要协作对象：通过 `Controllers::IMapController`、交互器、系统控制器等接口协作。
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "ui/CocosGUI.h"
#include "Game/Inventory.h"
#include "Game/GameConfig.h"
#include "Game/View/PlayerView.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/Input/PlayerController.h"
#include "Controllers/Map/FarmMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Interact/FarmInteractor.h"
#include "Controllers/Systems/AnimalSystem.h"
#include "Controllers/NPC/RobinNpcController.h"
#include "Scenes/SceneBase.h"

class FarmScene : public SceneBase {
public:
    // 创建农场场景实例。
    static cocos2d::Scene* createScene();

    // 初始化农场场景。
    virtual bool init() override;

    CREATE_FUNC(FarmScene);
    
    // 设置出生点为农场入口外侧（供其它场景返回时使用）。
    void setSpawnAtFarmEntrance();

    // 设置出生点为 `DoorToMine` 对象层中心（矿洞返回农场）。
    void setSpawnAtFarmMineDoor();

    // 设置出生点为 `DoorToRoom` 对象层中心（房间返回农场）。
    void setSpawnAtFarmRoomDoor();

    // 设置出生点为 `DoorToBeach` 对象层中心（沙滩返回农场）。
    void setSpawnAtFarmBeachDoor();

    // 设置出生点为 `DoorToTown` 对象层中心（城镇返回农场）。
    void setSpawnAtFarmTownDoor();

private:
    Controllers::FarmMapController* _farmMap = nullptr;
    Controllers::FarmInteractor* _interactor = nullptr;
    Controllers::AnimalSystem* _animalSystem = nullptr;
    Controllers::RobinNpcController* _robinNpc = nullptr;

    // 创建并返回农场地图控制器（以接口形式对 SceneBase 暴露）。
    Controllers::IMapController* createMapController(cocos2d::Node* worldNode) override;

    // 根据当前出生点配置设置玩家初始位置。
    void positionPlayerInitial() override;

    // 处理空格按键行为（通常为交互/工具触发入口）。
    void onSpacePressed() override;

    // 获取门交互提示文本。
    const char* doorPromptText() const override;

protected:
    // 处理键盘按键事件并转发到对应模块。
    void onKeyPressedHook(cocos2d::EventKeyboard::KeyCode code) override;

    // 处理鼠标按下事件并转发到对应模块。
    void onMouseDown(cocos2d::EventMouse* e) override;
};
