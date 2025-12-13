#include "Scenes/TownScene.h"
#include "Scenes/FarmScene.h"
#include "Game/Map/TownMap.h"
#include "Game/GameConfig.h"

using namespace cocos2d;
using namespace Controllers;

Scene* TownScene::createScene() { return TownScene::create(); }

bool TownScene::init() {
    if (!SceneBase::initBase(3.0f, true, true, true)) return false;
    _npcController = new TownNpcController(_townMap, _worldNode, _uiController, _inventory);
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
    return true;
}

IMapController* TownScene::createMapController(Node* worldNode) {
    auto map = Game::TownMap::create("Maps/town/town.tmx");
    _townMap = new TownMapController(map, worldNode);
    return _townMap;
}

void TownScene::positionPlayerInitial() {
    if (!_townMap || !_player) return;
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
