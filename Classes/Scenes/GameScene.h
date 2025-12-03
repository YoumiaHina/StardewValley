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

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(GameScene);

    void update(float dt) override;
    // 供其他场景调用：将玩家出生在农场门外侧
    void setSpawnAtFarmEntrance();

private:
    // 世界容器与核心对象
    cocos2d::Node* _worldNode = nullptr;
    Game::PlayerAppearance* _player = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;

    // 控制器与系统
    Controllers::PlayerController* _playerController = nullptr;
    Controllers::FarmMapController* _mapController = nullptr;
    Controllers::UIController* _uiController = nullptr;
    Controllers::ToolSystem* _toolSystem = nullptr;
    Controllers::GameStateController* _stateController = nullptr;
    Controllers::FarmInteractor* _interactor = nullptr;
};
