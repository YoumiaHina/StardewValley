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
#include "Scenes/BeachScene.h"
#include "Scenes/TownScene.h"
#include "Controllers/Managers/AudioManager.h"
#include "Controllers/Systems/FishingController.h"
#include "Game/Cheat.h"
#include "Controllers/Input/PlayerController.h"
#include "Controllers/Systems/AnimalSystem.h"
#include "Controllers/Map/FarmMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Interact/FarmInteractor.h"

USING_NS_CC;

Scene* FarmScene::createScene() { return FarmScene::create(); }

bool FarmScene::init() {
    if (!initBase(/*worldScale*/3.0f, /*buildCraftPanel*/true, /*enableToolOnSpace*/true, /*enableToolOnLeftClick*/true)) return false;
    Managers::AudioManager::getInstance().playBackgroundFor(Managers::SceneZone::Farm);
    _animalSystem = new Controllers::AnimalSystem(_mapController, _worldNode);
    if (_stateController) {
        _stateController->setAnimalSystem(_animalSystem);
    }
    _interactor = new Controllers::FarmInteractor(_inventory, _mapController, _uiController, _cropSystem, _animalSystem,
        [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); },
        [this]() -> Vec2 { return _playerController ? _playerController->lastDir() : Vec2(0,-1); });
    _robinNpc = new Controllers::RobinNpcController(_farmMap, _worldNode, _uiController, _inventory, _animalSystem);
    if (_interactor && _robinNpc) {
        _interactor->setNpcController(_robinNpc);
    }
    _fishing = new Controllers::FishingController(_mapController, _inventory, _uiController, this, _worldNode);
    addUpdateCallback([this](float dt){ if (_fishing) _fishing->update(dt); });
    addUpdateCallback([this](float dt){ if (_animalSystem) _animalSystem->update(dt); });
    addUpdateCallback([this](float){ if (_robinNpc && _player) _robinNpc->update(_player->getPosition()); });
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
    if (_uiController && _animalSystem) {
        _uiController->setAnimalStoreHandler([this](Game::AnimalType type) -> bool {
            if (!_animalSystem || !_player || !_mapController) return false;
            float s = static_cast<float>(GameConfig::TILE_SIZE);
            Vec2 pos = _player->getPosition();
            Vec2 dir = _playerController ? _playerController->lastDir() : Vec2(0, -1);
            if (dir.lengthSquared() < 1e-3f) dir = Vec2(0, -1);
            dir.normalize();
            Vec2 spawnPos = pos + dir * s;
            long long price = Game::animalPrice(type);
            return _animalSystem->buyAnimal(type, spawnPos, price);
        });
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

void FarmScene::setSpawnAtFarmBeachDoor() {
    if (!_player || !_mapController) return;
    auto pos = _mapController->farmBeachDoorSpawnPos();
    if (pos != Vec2::ZERO) {
        _player->setPosition(pos);
    }
}

void FarmScene::setSpawnAtFarmTownDoor() {
    if (!_player || !_mapController) return;
    auto pos = _mapController->farmTownDoorSpawnPos();
    if (pos != Vec2::ZERO) {
        _player->setPosition(pos);
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
    } else if (act == Controllers::FarmInteractor::SpaceAction::EnterBeach) {
        auto beach = BeachScene::createScene();
        auto trans = TransitionFade::create(0.6f, beach);
        Director::getInstance()->replaceScene(trans);
    } else if (act == Controllers::FarmInteractor::SpaceAction::EnterTown) {
        auto town = TownScene::createScene();
        auto trans = TransitionFade::create(0.6f, town);
        Director::getInstance()->replaceScene(trans);
    }
}

const char* FarmScene::doorPromptText() const { return "Press Space to Enter"; }

void FarmScene::onKeyPressedHook(EventKeyboard::KeyCode code) {
    if (code == EventKeyboard::KeyCode::KEY_K) {
        auto mine = MineScene::create();
        auto trans = TransitionFade::create(0.6f, mine);
        Director::getInstance()->replaceScene(trans);
    } else if (code == EventKeyboard::KeyCode::KEY_G) {
        if (!_player || !_mapController || !_animalSystem) return;
        float s = static_cast<float>(GameConfig::TILE_SIZE);
        Vec2 pos = _player->getPosition();
        Vec2 dir = _playerController ? _playerController->lastDir() : Vec2(0, -1);
        if (dir.lengthSquared() < 1e-3f) dir = Vec2(0, -1);
        dir.normalize();
        Vec2 left(-dir.y, dir.x);
        Vec2 right(dir.y, -dir.x);
        Vec2 front = pos + dir * s;
        Vec2 leftFront = front + left * s;
        Vec2 rightFront = front + right * s;
        _animalSystem->spawnAnimal(Game::AnimalType::Chicken, leftFront);
        _animalSystem->spawnAnimal(Game::AnimalType::Cow, front);
        _animalSystem->spawnAnimal(Game::AnimalType::Sheep, rightFront);
    }
}

void FarmScene::onMouseDown(cocos2d::EventMouse* e) {
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
    if (_interactor) _interactor->onLeftClick();
}
