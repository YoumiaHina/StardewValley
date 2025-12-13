/**
 * FarmScene: 精简场景，仅负责：创建场景、加载模块、调度更新、分发输入事件、控制场景切换。
 */
#include "Scenes/FarmScene.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Game/Inventory.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Tool/ToolFactory.h"
#include "Game/Tool/FishingRod.h"
#include "Scenes/RoomScene.h"
#include "Scenes/MineScene.h"
#include "Managers/AudioManager.h"
#include "Controllers/Systems/FishingController.h"
#include "Game/Cheat.h"
#include "Controllers/Input/PlayerController.h"
#include "Controllers/Map/FarmMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Interact/FarmInteractor.h"

USING_NS_CC;

Scene* FarmScene::createScene() { return FarmScene::create(); }

bool FarmScene::init() {
    if (!initBase(/*worldScale*/3.0f, /*buildCraftPanel*/true, /*enableToolOnSpace*/true, /*enableToolOnLeftClick*/true)) return false;
    Managers::AudioManager::getInstance().playBackgroundFor(Managers::SceneZone::Farm);
    _interactor = new Controllers::FarmInteractor(_inventory, _mapController, _uiController, _cropSystem,
        [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); },
        [this]() -> Vec2 { return _playerController ? _playerController->lastDir() : Vec2(0,-1); });
    _fishing = new Controllers::FishingController(_mapController, _inventory, _uiController, this, _worldNode);
    addUpdateCallback([this](float dt){ if (_fishing) _fishing->update(dt); });
    if (_inventory && _fishing) {
        for (std::size_t i = 0; i < _inventory->size(); ++i) {
            auto tb = _inventory->toolAtMutable(i);
            if (tb && tb->kind() == Game::ToolKind::FishingRod) {
                auto rod = dynamic_cast<Game::FishingRod*>(tb);
                if (rod) { rod->setFishingStarter([this](const Vec2& pos){ if (_fishing) _fishing->startAt(pos); }); }
                break;
            }
        }
    }
    if (_fishing) {
        _fishing->setMovementLocker([this](bool locked){ if (_playerController) _playerController->setMovementLocked(locked); });
    }
    return true;
}

void FarmScene::setSpawnAtFarmEntrance() {
    if (!_player || !_mapController) return;
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    // 使用门矩形中点向上偏移 1 格
    // 简化：直接取门矩形由 map 内部维护；此处以玩家当前位置替代具体门口位置以避免强耦合
    _player->setPosition(_player->getPosition() + Vec2(0, s));
}

void FarmScene::setSpawnAtFarmMineDoor() {
    if (!_player || !_mapController) return;
    auto pos = _mapController->farmMineDoorSpawnPos();
    if (pos != Vec2::ZERO) {
        _player->setPosition(pos);
    }
}

void FarmScene::setSpawnAtFarmRoomDoor() {
    if (!_player || !_mapController) return;
    auto pos = _mapController->farmRoomDoorSpawnPos();
    if (pos != Vec2::ZERO) {
        _player->setPosition(pos + Vec2(0, -3.0f));
    }
}

// SceneBase overrides
Controllers::IMapController* FarmScene::createMapController(Node* worldNode) {
    _farmMap = new Controllers::FarmMapController(worldNode);
    _farmMap->init();
    return _farmMap;
}

void FarmScene::positionPlayerInitial() {
    _player->setPosition(_mapController->tileToWorld(GameConfig::MAP_COLS / 2, GameConfig::MAP_ROWS / 2));
}

void FarmScene::onSpacePressed() {
    auto act = _interactor ? _interactor->onSpacePressed() : Controllers::FarmInteractor::SpaceAction::None;
    if (act == Controllers::FarmInteractor::SpaceAction::EnterHouse) {
        auto room = RoomScene::create();
        room->setSpawnInsideDoor();
        auto trans = TransitionFade::create(0.6f, room);
        Director::getInstance()->replaceScene(trans);
    } else if (act == Controllers::FarmInteractor::SpaceAction::EnterMine) {
        auto mine = MineScene::create();
        auto trans = TransitionFade::create(0.6f, mine);
        Director::getInstance()->replaceScene(trans);
    }
}

const char* FarmScene::doorPromptText() const { return "Press Space to Enter"; }

void FarmScene::onKeyPressedHook(EventKeyboard::KeyCode code) {
    if (code == EventKeyboard::KeyCode::KEY_K) {
        auto mine = MineScene::create();
        auto trans = TransitionFade::create(0.6f, mine);
        Director::getInstance()->replaceScene(trans);
    }
}
