/**
 * MineScene：矿洞场景（继承 SceneBase），负责矿洞地图的场景骨架与事件转发。
 * - 职责边界：只做场景初始化/输入事件转发/场景切换触发，不承载战斗与怪物等业务规则。
 * - 主要协作对象：通过 `Controllers::IMapController`、战斗/怪物/电梯等控制接口协作。
 */
#pragma once

#include "Scenes/SceneBase.h"
#include "Controllers/Map/MineMapController.h"
#include "Controllers/Mine/MonsterSystem.h"
#include "Controllers/Mine/CombatSystem.h"
#include "Controllers/Interact/MineInteractor.h"
#include "Controllers/Mine/ElevatorSystem.h"
namespace Controllers { class ChestInteractor; }

class MineScene : public SceneBase {
public:
    // 创建矿洞场景实例。
    static cocos2d::Scene* createScene();

    // 初始化矿洞场景。
    virtual bool init() override;
    CREATE_FUNC(MineScene);

private:
    Controllers::MineMapController* _map = nullptr;
    Controllers::MineMonsterController* _monsters = nullptr;
    Controllers::MineCombatController* _combat = nullptr;
    Controllers::MineInteractor* _interactor = nullptr;
    Controllers::MineElevatorController* _elevator = nullptr;
    Controllers::ChestInteractor* _chestInteractor = nullptr;
    bool _inTransition = false;

    // 创建并返回矿洞地图控制器（以接口形式对 SceneBase 暴露）。
    Controllers::IMapController* createMapController(cocos2d::Node* worldNode) override;

    // 设置玩家在矿洞的初始出生位置。
    void positionPlayerInitial() override;

    // 处理空格按键行为（通常为交互/工具触发入口）。
    void onSpacePressed() override;

    // 获取门交互提示文本。
    const char* doorPromptText() const override;

protected:
    // 处理鼠标按下事件（矿洞主要用于攻击/使用物品入口）并转发到对应模块。
    void onMouseDown(cocos2d::EventMouse* e) override;

    // 处理键盘按键事件并转发到对应模块。
    void onKeyPressedHook(cocos2d::EventKeyboard::KeyCode code) override;

    // 判断场景是否阻止玩家移动（用于过场/切层等状态）。
    bool isMovementBlockedByScene() const override;
};
