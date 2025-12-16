/**
 * FarmScene: Main game scene placeholder.
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "ui/CocosGUI.h"
#include "Game/Inventory.h"
#include "Game/GameConfig.h"
#include "Game/View/PlayerView.h"
#include "Controllers/IMapController.h"
#include "Controllers/Input/PlayerController.h"
#include "Controllers/Map/FarmMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Interact/FarmInteractor.h"
#include "Controllers/Systems/FishingController.h"
#include "Controllers/Systems/AnimalSystem.h"
#include "Controllers/NPC/RobinNpcController.h"
#include "Scenes/SceneBase.h"

class FarmScene : public SceneBase {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(FarmScene);
    
    // 供其他场景调用：将玩家出生在农场门外侧
    void setSpawnAtFarmEntrance();
    // 供矿洞返回：将玩家出生在 DoorToMine 对象层中心
    void setSpawnAtFarmMineDoor();
    // 供房间返回：将玩家出生在 DoorToRoom 对象层中心
    void setSpawnAtFarmRoomDoor();
    // 供沙滩返回：将玩家出生在 DoorToBeach 对象层中心
    void setSpawnAtFarmBeachDoor();
    // 供城镇返回：将玩家出生在 DoorToTown 对象层中心
    void setSpawnAtFarmTownDoor();

private:
    Controllers::FarmMapController* _farmMap = nullptr;
    Controllers::FarmInteractor* _interactor = nullptr;
    Controllers::FishingController* _fishing = nullptr;
    Controllers::AnimalSystem* _animalSystem = nullptr;
    Controllers::RobinNpcController* _robinNpc = nullptr;

    // SceneBase overrides
    Controllers::IMapController* createMapController(cocos2d::Node* worldNode) override;
    void positionPlayerInitial() override;
    void onSpacePressed() override;
    const char* doorPromptText() const override;

protected:
    void onKeyPressedHook(cocos2d::EventKeyboard::KeyCode code) override;
    void onMouseDown(cocos2d::EventMouse* e) override;
};
