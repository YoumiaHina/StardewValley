/**
 * MineScene: 矿洞场景（继承 SceneBase），严格保持极简：组合模块、事件转发、场景切换。
 */
#pragma once

#include "Scenes/SceneBase.h"
#include "Controllers/Map/MineMapController.h"
#include "Controllers/MineMonsterController.h"
#include "Controllers/MineMiningController.h"
#include "Controllers/MineCombatController.h"
#include "Controllers/Interact/MineInteractor.h"
#include "Controllers/MineElevatorController.h"

class MineScene : public SceneBase {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(MineScene);

private:
    Controllers::MineMapController* _map = nullptr;
    Controllers::MineMonsterController* _monsters = nullptr;
    Controllers::MineMiningController* _mining = nullptr;
    Controllers::MineCombatController* _combat = nullptr;
    Controllers::MineInteractor* _interactor = nullptr;
    Controllers::MineElevatorController* _elevator = nullptr;
    bool _inTransition = false;

    // SceneBase overrides
    Controllers::IMapController* createMapController(cocos2d::Node* worldNode) override;
    void positionPlayerInitial() override;
    void onSpacePressed() override;
    const char* doorPromptText() const override;

protected:
    // 额外事件转发：左键攻击/右键物品（此处主要处理左键）
    void onMouseDown(cocos2d::EventMouse* e) override;
    void onKeyPressedHook(cocos2d::EventKeyboard::KeyCode code) override;
};
