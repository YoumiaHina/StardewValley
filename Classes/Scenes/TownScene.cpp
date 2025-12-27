#include "Scenes/TownScene.h"
#include "Scenes/FarmScene.h"
#include "Game/Map/TownMap.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"
#include "Controllers/Interact/ChestInteractor.h"
#include "Controllers/NPC/AbigailNpcController.h"
#include "Controllers/NPC/PierreNpcController.h"

using namespace cocos2d;
using namespace Controllers;

static const char* seasonKeyFromIndex(int seasonIndex) {
    switch (seasonIndex) {
        case 0: return "spring";
        case 1: return "summer";
        case 2: return "fall";
        case 3: return "winter";
        default: return "spring";
    }
}

Scene* TownScene::createScene() { return TownScene::create(); }

bool TownScene::init() {
    if (!SceneBase::initBase(3.0f, true, true, true)) return false;
    auto& ws = Game::globalState();
    ws.lastScene = static_cast<int>(Game::SceneKind::Town);
    _npcController = new NpcController(_uiController);
    _npcController->add(std::make_unique<AbigailNpcController>(_townMap, _worldNode, _uiController, _inventory, _npcController->dialogue()));
    _npcController->add(std::make_unique<PierreNpcController>(_townMap, _worldNode, _uiController, _inventory));
    addUpdateCallback([this](float) {
        if (_npcController && _player) {
            _npcController->update(_player->getPosition());
        }
    });
    _interactor.setMap(_townMap);
    _interactor.setGetPlayerPos([this]() { return _player ? _player->getPosition() : Vec2::ZERO; });
    _interactor.setNpcController(_npcController);
    _interactor.setUI(_uiController);
    _interactor.setInventory(_inventory);
    auto mouseNpc = EventListenerMouse::create();
    mouseNpc->onMouseDown = [this](EventMouse* e) {
        if (!_npcController) return;
        if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_RIGHT) return;
        _npcController->handleRightClick(e);
    };
    Director::getInstance()->getEventDispatcher()
        ->addEventListenerWithSceneGraphPriority(mouseNpc, this);

    auto* townMap = _townMap;
    if (townMap && _uiController && _inventory) {
        auto* furnace = townMap->furnaceController();
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

TownScene::~TownScene() {
    delete _chestInteractor;
    _chestInteractor = nullptr;
    delete _npcController;
    _npcController = nullptr;
}

IMapController* TownScene::createMapController(Node* worldNode) {
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
    const std::string tmxPath = "Maps/town/" + key + "_town.tmx";
    auto map = Game::TownMap::create(tmxPath);
    _townMap = new TownMapController(map, worldNode);
    return _townMap;
}

void TownScene::positionPlayerInitial() {
    if (!_townMap || !_player) return;
    auto& ws = Game::globalState();
    if (ws.lastScene == static_cast<int>(Game::SceneKind::Town) &&
        (ws.lastPlayerX != 0.0f || ws.lastPlayerY != 0.0f)) {
        _player->setPosition(Vec2(ws.lastPlayerX, ws.lastPlayerY));
        return;
    }
    auto center = _townMap->getTownMap()->doorToFarmCenter();
    Vec2 p = center != Vec2::ZERO ? center + Vec2(0, 28.0f) : Vec2(_townMap->getContentSize().width*0.5f, _townMap->getContentSize().height*0.5f);
    _player->setPosition(p);
}

void TownScene::onSpacePressed() {
    auto act = _interactor.onSpacePressed();
    if (act == TownInteractor::SpaceAction::EnterFarm) {
        auto next = FarmScene::create();
        next->setSpawnAtFarmTownDoor();
        Director::getInstance()->replaceScene(TransitionFade::create(0.6f, next));
    }
}

const char* TownScene::doorPromptText() const { return "Press Space to Enter Farm"; }

void TownScene::onMouseDown(EventMouse* e) {
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

    if (_townMap && _player) {
        auto* furnace = _townMap->furnaceController();
        if (furnace) {
            Vec2 p = _player->getPosition();
            Vec2 lastDir = _playerController ? _playerController->lastDir() : Vec2(0,-1);
            furnace->interactAt(p, lastDir);
        }
    }
}

void TownScene::onKeyPressedHook(EventKeyboard::KeyCode code) {
    (void)code;
}
