/**
 * RoomScene: 玩家出生在室内，走到门口自动切换到农场（FarmScene）。
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "Game/Inventory.h"
#include "ui/CocosGUI.h"
#include "Game/View/PlayerView.h"
#include "Controllers/IMapController.h"
#include "Controllers/Input/PlayerController.h"
#include "Controllers/Map/RoomMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Interact/RoomInteractor.h"
#include "Scenes/SceneBase.h"

class RoomScene : public SceneBase {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(RoomScene);
    // 设置在门内侧的出生点（从农场返回时）
    void setSpawnInsideDoor();

private:
    Controllers::RoomMapController* _roomMap = nullptr;
    Controllers::RoomInteractor* _interactor = nullptr;

    // SceneBase overrides
    Controllers::IMapController* createMapController(cocos2d::Node* worldNode) override;
    void positionPlayerInitial() override;
    void onSpacePressed() override;
    const char* doorPromptText() const override;

protected:
    void onKeyPressedHook(cocos2d::EventKeyboard::KeyCode code) override;
};
