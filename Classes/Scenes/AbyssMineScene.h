/**
 * AbyssMineScene: 深渊矿洞场景（继承 SceneBase），严格保持极简：组合模块、事件转发、场景切换。
 */
#pragma once

#include "Scenes/SceneBase.h"
#include "Controllers/Map/AbyssMapController.h"
#include "Controllers/AbyssMonsterController.h"
#include "Controllers/AbyssMiningController.h"
#include "Controllers/AbyssCombatController.h"
#include "Controllers/Interact/AbyssInteractor.h"
#include "Controllers/AbyssElevatorController.h"

class AbyssMineScene : public SceneBase {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(AbyssMineScene);

private:
    Controllers::AbyssMapController* _map = nullptr;
    Controllers::AbyssMonsterController* _monsters = nullptr;
    Controllers::AbyssMiningController* _mining = nullptr;
    Controllers::AbyssCombatController* _combat = nullptr;
    Controllers::AbyssInteractor* _interactor = nullptr;
    Controllers::AbyssElevatorController* _elevator = nullptr;
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
