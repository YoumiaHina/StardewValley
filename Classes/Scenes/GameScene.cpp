/**
 * GameScene: 精简场景，仅负责：创建场景、加载模块、调度更新、分发输入事件、控制场景切换。
 */
#include "Scenes/GameScene.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Game/Inventory.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Tool.h"
#include "Scenes/RoomScene.h"
#include "Game/Cheat.h"
#include "Controllers/PlayerController.h"
#include "Controllers/FarmMapController.h"
#include "Controllers/UIController.h"
#include "Controllers/ToolSystem.h"
#include "Controllers/GameStateController.h"
#include "Controllers/FarmInteractor.h"

USING_NS_CC;

Scene* GameScene::createScene() { return GameScene::create(); }

bool GameScene::init() {
    if (!initBase(/*worldScale*/3.0f, /*buildCraftPanel*/true, /*enableToolOnSpace*/true, /*enableToolOnLeftClick*/true)) return false;
    _interactor = new Controllers::FarmInteractor(_inventory, _mapController, _uiController,
        [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); });
    return true;
}

void GameScene::setSpawnAtFarmEntrance() {
    if (!_player || !_mapController) return;
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    // 使用门矩形中点向上偏移 1 格
    // 简化：直接取门矩形由 map 内部维护；此处以玩家当前位置替代具体门口位置以避免强耦合
    _player->setPosition(_player->getPosition() + Vec2(0, s));
}

// SceneBase overrides
Controllers::IMapController* GameScene::createMapController(Node* worldNode) {
    _farmMap = new Controllers::FarmMapController(worldNode);
    _farmMap->init();
    return _farmMap;
}

void GameScene::positionPlayerInitial() {
    _player->setPosition(_mapController->tileToWorld(GameConfig::MAP_COLS / 2, GameConfig::MAP_ROWS / 2));
}

void GameScene::onSpacePressed() {
    auto act = _interactor ? _interactor->onSpacePressed() : Controllers::FarmInteractor::SpaceAction::None;
    if (act == Controllers::FarmInteractor::SpaceAction::EnterHouse) {
        auto room = RoomScene::create();
        room->setSpawnInsideDoor();
        auto trans = TransitionFade::create(0.6f, room);
        Director::getInstance()->replaceScene(trans);
    }
}

const char* GameScene::doorPromptText() const { return "Press Space to Enter House"; }