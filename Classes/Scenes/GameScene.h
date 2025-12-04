/**
 * GameScene: Main game scene placeholder.
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "ui/CocosGUI.h"
#include "Game/Inventory.h"
#include "Game/GameConfig.h"
#include "Game/PlayerAppearance.h"
#include "Controllers/IMapController.h"
#include "Controllers/PlayerController.h"
#include "Controllers/FarmMapController.h"
#include "Controllers/ToolSystem.h"
#include "Controllers/UIController.h"
#include "Controllers/GameStateController.h"
#include "Controllers/FarmInteractor.h"
#include "Scenes/SceneBase.h"

class GameScene : public SceneBase {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(GameScene);
    
    // 供其他场景调用：将玩家出生在农场门外侧
    void setSpawnAtFarmEntrance();

private:
    Controllers::FarmMapController* _farmMap = nullptr;
    Controllers::FarmInteractor* _interactor = nullptr;

    // SceneBase overrides
    Controllers::IMapController* createMapController(cocos2d::Node* worldNode) override;
    void positionPlayerInitial() override;
    void onSpacePressed() override;
    const char* doorPromptText() const override;

protected:
    void onKeyPressedHook(cocos2d::EventKeyboard::KeyCode code) override;
};
