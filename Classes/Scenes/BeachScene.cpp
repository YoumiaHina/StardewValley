#include "Scenes/BeachScene.h"
#include "Scenes/FarmScene.h"
#include "Game/Map/BeachMap.h"
#include "Game/GameConfig.h"

using namespace cocos2d;
using namespace Controllers;

Scene* BeachScene::createScene() { return BeachScene::create(); }

bool BeachScene::init() {
    if (!SceneBase::initBase(3.0f, true, true, true)) return false;
    _interactor.setMap(_beachMap);
    _interactor.setInventory(_inventory.get());
    _interactor.setUI(_uiController);
    _interactor.setGetPlayerPos([this]() { return _player ? _player->getPosition() : Vec2::ZERO; });
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
        auto next = FarmScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.6f, next));
    }
}

const char* BeachScene::doorPromptText() const { return "Press Space to Enter Farm"; }
