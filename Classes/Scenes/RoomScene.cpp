/**
 * RoomScene: 精简场景，仅负责组合模块、更新调度与事件分发。
 */
#include "Scenes/RoomScene.h"
#include "Scenes/GameScene.h"
#include "Scenes/AbyssMineScene.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Game/Inventory.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Controllers/PlayerController.h"
#include "Controllers/RoomMapController.h"
#include "Controllers/UIController.h"
#include "Controllers/GameStateController.h"
#include "Controllers/RoomInteractor.h"
#include "Game/Cheat.h"

USING_NS_CC;

Scene* RoomScene::createScene() { return RoomScene::create(); }

bool RoomScene::init() {
    if (!initBase(/*worldScale*/2.0f, /*buildCraftPanel*/false, /*enableToolOnSpace*/false, /*enableToolOnLeftClick*/false)) return false;
    _interactor = new Controllers::RoomInteractor(_inventory, _mapController, _uiController,
        [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); });
    return true;
}

void RoomScene::setSpawnInsideDoor() {
    if (!_player || !_roomMap) return;
    float offsetY = 28.0f;
    Vec2 spawn(_roomMap->doorRect().getMidX(), _roomMap->doorRect().getMinY() + _roomMap->doorRect().size.height + offsetY);
    _player->setPosition(spawn);
}

// SceneBase overrides
Controllers::IMapController* RoomScene::createMapController(Node* worldNode) {
    _roomMap = new Controllers::RoomMapController(worldNode);
    _roomMap->init();
    return _roomMap;
}

void RoomScene::positionPlayerInitial() {
    _player->setPosition(Vec2(_roomMap->roomRect().getMidX(), _roomMap->roomRect().getMidY() + 80));
}

void RoomScene::onSpacePressed() {
    auto act = _interactor ? _interactor->onSpacePressed() : Controllers::RoomInteractor::SpaceAction::None;
    if (act == Controllers::RoomInteractor::SpaceAction::ExitHouse) {
        auto farm = GameScene::create();
        farm->setSpawnAtFarmEntrance();
        auto trans = TransitionFade::create(0.6f, farm);
        Director::getInstance()->replaceScene(trans);
    }
}

const char* RoomScene::doorPromptText() const { return "Press Space to Exit"; }

void RoomScene::onKeyPressedHook(EventKeyboard::KeyCode code) {
    if (code == EventKeyboard::KeyCode::KEY_K) {
        auto abyss = AbyssMineScene::create();
        auto trans = TransitionFade::create(0.6f, abyss);
        Director::getInstance()->replaceScene(trans);
    }
}