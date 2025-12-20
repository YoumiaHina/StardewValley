#include "Scenes/BeachScene.h"
#include "Scenes/FarmScene.h"
#include "Game/Map/BeachMap.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"
#include "Controllers/Interact/ChestInteractor.h"
#include "Controllers/NPC/WillyNpcController.h"
#include "Controllers/NPC/NpcControllerBase.h"
#include "string"

using namespace cocos2d;
using namespace Controllers;

Scene* BeachScene::createScene() { return BeachScene::create(); }

bool BeachScene::init() {
    if (!SceneBase::initBase(3.0f, true, true, true)) return false;
    auto& ws = Game::globalState();
    ws.lastScene = static_cast<int>(Game::SceneKind::Beach);
    _interactor.setMap(_beachMap);
    _interactor.setInventory(_inventory.get());
    _interactor.setUI(_uiController);
    _interactor.setGetPlayerPos([this]() { return _player ? _player->getPosition() : Vec2::ZERO; });
    if (!_npcController && _uiController) {
        _npcController = new NpcController(_uiController);
    }
    if (_npcController && _beachMap && _worldNode && _uiController) {
        _npcController->add(std::make_unique<WillyNpcController>(
            _beachMap,
            _worldNode,
            _uiController,
            _inventory,
            _npcController->dialogue()));
    }
    if (_npcController) {
        addUpdateCallback([this](float) {
            if (_npcController && _player) {
                _npcController->update(_player->getPosition());
            }
        });
    }
    auto* beachMap = _beachMap;
    if (beachMap && _uiController && _inventory) {
        auto* furnace = beachMap->furnaceController();
        if (furnace) {
            furnace->bindContext(_mapController, _uiController, _inventory);
            furnace->syncLoad();
            addUpdateCallback([furnace](float dt) {
                furnace->update(dt);
            });
        }
    }
    return true;
}

IMapController* BeachScene::createMapController(Node* worldNode) {
    const auto& ws = Game::globalState();
    std::string key;
    switch (ws.seasonIndex)
    {
        case 0:
			key = "spring";
			break;
		case 1:
			key = "summer";
			break;
        case 2:
            key = "fall";
            break;
        case 3:
			key = "winter";
			break;
        default:
			key = "spring";
            break;
    }
	const std::string tmxPath = "Maps/beach/" + key + "_beach.tmx";
    auto map = Game::BeachMap::create(tmxPath);
    _beachMap = new BeachMapController(map, worldNode);
    return _beachMap;
}

void BeachScene::positionPlayerInitial() {
    if (!_beachMap || !_player) return;
    auto& ws = Game::globalState();
    if (ws.lastScene == static_cast<int>(Game::SceneKind::Beach) &&
        (ws.lastPlayerX != 0.0f || ws.lastPlayerY != 0.0f)) {
        _player->setPosition(Vec2(ws.lastPlayerX, ws.lastPlayerY));
        return;
    }
    auto center = _beachMap->getBeachMap()->doorToFarmCenter();
    Vec2 p = center != Vec2::ZERO ? center + Vec2(0, 28.0f) : Vec2(_beachMap->getContentSize().width*0.5f, _beachMap->getContentSize().height*0.5f);
    _player->setPosition(p);
}

void BeachScene::onSpacePressed() {
    auto act = _interactor.onSpacePressed();
    if (act == BeachInteractor::SpaceAction::EnterFarm) {
        auto next = FarmScene::create();
        next->setSpawnAtFarmBeachDoor();
        Director::getInstance()->replaceScene(TransitionFade::create(0.6f, next));
        return;
    }
    if (!_npcController || !_player) return;
    if (_npcController->advanceDialogueIfActive()) return;
    _npcController->handleTalkAt(_player->getPosition());
}

const char* BeachScene::doorPromptText() const { return "Press Space to Enter Farm"; }

void BeachScene::onMouseDown(EventMouse* e) {
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
        if (_npcController && _npcController->handleRightClick(e)) return;
    }
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
    if (!_chestInteractor && _inventory && _mapController && _uiController) {
        _chestInteractor = new Controllers::ChestInteractor(
            _inventory,
            _mapController,
            _uiController,
            [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); },
            [this]() -> Vec2 { return _playerController ? _playerController->lastDir() : Vec2(0,-1); });
    }
    if (_chestInteractor) _chestInteractor->onLeftClick();

    if (_beachMap && _player) {
        auto* furnace = _beachMap->furnaceController();
        if (furnace) {
            Vec2 p = _player->getPosition();
            Vec2 lastDir = _playerController ? _playerController->lastDir() : Vec2(0,-1);
            furnace->interactAt(p, lastDir);
        }
    }
}
