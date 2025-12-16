#include "Scenes/BeachScene.h"
#include "Scenes/FarmScene.h"
#include "Game/Map/BeachMap.h"
#include "Game/GameConfig.h"
#include "Controllers/Systems/FishingController.h"
#include "Game/Tool/FishingRod.h"
#include "Controllers/Interact/ChestInteractor.h"

using namespace cocos2d;
using namespace Controllers;

Scene* BeachScene::createScene() { return BeachScene::create(); }

bool BeachScene::init() {
    if (!SceneBase::initBase(3.0f, true, true, true)) return false;
    _interactor.setMap(_beachMap);
    _interactor.setInventory(_inventory.get());
    _interactor.setUI(_uiController);
    _interactor.setGetPlayerPos([this]() { return _player ? _player->getPosition() : Vec2::ZERO; });
    _fishing = new Controllers::FishingController(_mapController,
                                                  _inventory,
                                                  _uiController,
                                                  this,
                                                  _worldNode);
    addUpdateCallback([this](float dt) {
        if (_fishing) _fishing->update(dt);
    });
    if (_inventory && _fishing) {
        for (std::size_t i = 0; i < _inventory->size(); ++i) {
            auto tb = _inventory->toolAtMutable(i);
            if (tb && tb->kind() == Game::ToolKind::FishingRod) {
                auto rod = dynamic_cast<Game::FishingRod*>(tb);
                if (rod) {
                    rod->setFishingStarter(
                        [this](const Vec2& pos) {
                            if (_fishing) _fishing->startAt(pos);
                        });
                }
                break;
            }
        }
    }
    if (_fishing && _playerController) {
        _fishing->setMovementLocker(
            [this](bool locked) {
                if (_playerController) _playerController->setMovementLocked(locked);
            });
    }
    return true;
}

IMapController* BeachScene::createMapController(Node* worldNode) {
    auto map = Game::BeachMap::create("Maps/beach/beach.tmx");
    _beachMap = new BeachMapController(map, worldNode);
    return _beachMap;
}

void BeachScene::positionPlayerInitial() {
    if (!_beachMap || !_player) return;
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
    }
}

const char* BeachScene::doorPromptText() const { return "Press Space to Enter Farm"; }

void BeachScene::onMouseDown(EventMouse* e) {
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
}
