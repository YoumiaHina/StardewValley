/**
 * RoomScene: 玩家出生在室内，走到门口自动切换到农场（GameScene）。
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "Game/Inventory.h"
#include "ui/CocosGUI.h"
#include "Game/PlayerAppearance.h"
#include "Controllers/IMapController.h"
#include "Controllers/PlayerController.h"
#include "Controllers/RoomMapController.h"
#include "Controllers/UIController.h"
#include "Controllers/GameStateController.h"
#include "Controllers/RoomInteractor.h"

class RoomScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(RoomScene);

    void update(float dt) override;
    // 设置在门内侧的出生点（从农场返回时）
    void setSpawnInsideDoor();

private:
    cocos2d::Node* _worldNode = nullptr;
    Game::PlayerAppearance* _player = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;

    Controllers::PlayerController* _playerController = nullptr;
    Controllers::RoomMapController* _mapController = nullptr;
    Controllers::UIController* _uiController = nullptr;
    Controllers::GameStateController* _stateController = nullptr;
    Controllers::RoomInteractor* _interactor = nullptr;
};