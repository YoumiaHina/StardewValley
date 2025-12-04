#include "Scenes/AbyssMineScene.h"
#include "cocos2d.h"

USING_NS_CC;

cocos2d::Scene* AbyssMineScene::createScene() { return AbyssMineScene::create(); }

bool AbyssMineScene::init() {
    // 深渊矿洞：不启用空格工具、不启用左键工具；不显示 Craft 面板
    if (!initBase(/*worldScale*/2.5f, /*buildCraftPanel*/false, /*enableToolOnSpace*/false, /*enableToolOnLeftClick*/false)) return false;

    // 组合矿洞模块
    _map = static_cast<Controllers::AbyssMapController*>(_mapController);
    _map->generateFloor(1);
    _monsters = new Controllers::AbyssMonsterController(_map, _worldNode);
    _monsters->generateInitialWave();
    _mining = new Controllers::AbyssMiningController(_map, _worldNode);
    _mining->generateNodesForFloor();
    _combat = new Controllers::AbyssCombatController(_monsters, _mining, [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); });
    _interactor = new Controllers::AbyssInteractor(_map, [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); });
    _elevator = new Controllers::AbyssElevatorController(_map, _monsters, _mining, this);
    _elevator->buildPanel();
    // 矿洞 HUD：在能量条正上方构建血条（红色）
    if (_uiController) _uiController->buildHPBarAboveEnergy();

    // 注册模块更新
    addUpdateCallback([this](float dt){ if (_monsters) _monsters->update(dt); });
    addUpdateCallback([this](float dt){ if (_mining) _mining->update(dt); });
    addUpdateCallback([this](float dt){ if (_combat) _combat->update(dt); });

    return true;
}

Controllers::IMapController* AbyssMineScene::createMapController(Node* worldNode) {
    _map = new Controllers::AbyssMapController(worldNode);
    return _map;
}

void AbyssMineScene::positionPlayerInitial() {
    // 初始放置在地图中部偏上
    auto size = _map->getContentSize();
    _player->setPosition(Vec2(size.width * 0.5f, size.height * 0.65f));
}

void AbyssMineScene::onSpacePressed() {
    auto act = _interactor ? _interactor->onSpacePressed() : Controllers::AbyssInteractor::SpaceAction::None;
    if (act == Controllers::AbyssInteractor::SpaceAction::Descend) {
        // 重置当层状态
        if (_monsters) { _monsters->resetFloor(); _monsters->generateInitialWave(); }
        if (_mining) { _mining->generateNodesForFloor(); }
        // 将玩家放置在新楼层的入口附近
        _player->setPosition(_map->stairsWorldPos() + Vec2(0, 28));
    }
}

const char* AbyssMineScene::doorPromptText() const { return "Press Space to Descend"; }

void AbyssMineScene::onMouseDown(EventMouse* e) {
    if (_combat) _combat->onMouseDown(e);
}

void AbyssMineScene::onKeyPressedHook(EventKeyboard::KeyCode code) {
    if (code == EventKeyboard::KeyCode::KEY_E) {
        if (_elevator) _elevator->togglePanel();
    }
}