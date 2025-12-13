#include "Scenes/AbyssMineScene.h"
#include "cocos2d.h"
#include "Managers/AudioManager.h"
#include "Scenes/FarmScene.h"
#include "Game/Tool/ToolFactory.h"

USING_NS_CC;

cocos2d::Scene* AbyssMineScene::createScene() { return AbyssMineScene::create(); }

bool AbyssMineScene::init() {
    // 深渊矿洞：不启用空格工具、不启用左键工具；不显示 Craft 面板
    if (!initBase(/*worldScale*/3.0f, /*buildCraftPanel*/false, /*enableToolOnSpace*/false, /*enableToolOnLeftClick*/false)) return false;
    Managers::AudioManager::getInstance().playBackgroundFor(Managers::SceneZone::Abyss);

    // 组合矿洞模块
    _map = static_cast<Controllers::AbyssMapController*>(_mapController);
    // 入口楼层（零层）：加载 TMX 地图 Resources/Maps/mine/mine_0.tmx
    _map->loadEntrance();
    // 设置出生点到 Appear 对象层中心（若无则楼梯中心/fallback）
    if (_player) {
        _player->setPosition(_map->entranceSpawnPos());
        _player->setLocalZOrder(999); // 人物置于图层最上层
    }
    // 刷新热键栏以反映入口阶段可能发生的背包变化（如赠剑）
    if (_uiController) _uiController->refreshHotbar();
    // 第一次进入矿洞0层：仅赠送一次剑（即使之后丢弃也不再发放）
    {
        auto &ws = Game::globalState();
        if (!ws.grantedSwordAtEntrance && ws.inventory) {
            bool hasSword = false;
            for (std::size_t i = 0; i < ws.inventory->size(); ++i) {
                if (auto t = ws.inventory->toolAt(i)) {
                    if (t->kind() == Game::ToolKind::Sword) { hasSword = true; break; }
                }
            }
            bool inserted = false;
            if (!hasSword) {
                // 放到第一个空槽位
                for (std::size_t i = 0; i < ws.inventory->size(); ++i) {
                    if (ws.inventory->isEmpty(i)) {
                        ws.inventory->setTool(i, Game::makeTool(Game::ToolKind::Sword));
                        inserted = true;
                        if (_uiController) _uiController->refreshHotbar();
                        if (_player) _uiController->popTextAt(_player->getPosition(), "Got Sword!", cocos2d::Color3B::GREEN);
                        break;
                    }
                }
                if (!inserted && _player && _uiController) {
                    _uiController->popTextAt(_player->getPosition(), "Inventory Full", cocos2d::Color3B::RED);
                }
            }
            // 仅在成功放入或已拥有剑时标记为已赠送，避免“未实际获得却阻断后续赠送”
            if (inserted || hasSword) {
                ws.grantedSwordAtEntrance = true;
            }
        }
    }
    _monsters = new Controllers::AbyssMonsterController(_map, _worldNode);
    _mining = new Controllers::AbyssMiningController(_map, _worldNode);
    // 矿洞零层不刷怪、不生成矿点
    if (_map->currentFloor() > 0) {
        _monsters->generateInitialWave();
        _mining->generateNodesForFloor();
    }
    _combat = new Controllers::AbyssCombatController(_monsters, _mining, [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); });
    _interactor = new Controllers::AbyssInteractor(_map, [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); });
    _elevator = new Controllers::AbyssElevatorController(_map, _monsters, _mining, this);
    _elevator->buildPanel();
    // 电梯面板打开时锁定移动；跳转后更新楼层标签并定位到该层出生点
    _elevator->setMovementLocker([this](bool locked){ if (_playerController) _playerController->setMovementLocked(locked); });
    _elevator->setOnFloorChanged([this](int floor){
        if (_uiController) _uiController->setMineFloorNumber(floor);
        if (_player) _player->setPosition(_map->floorSpawnPos());
    });
    // 矿洞 HUD：在能量条正上方构建血条（红色）
    if (_uiController) {
        _uiController->buildHPBarAboveEnergy();
        _uiController->buildMineFloorLabel();
        _uiController->setMineFloorNumber(_map->currentFloor());
    }

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
    if (_inTransition) return;
    _inTransition = true;
    auto act = _interactor ? _interactor->onSpacePressed() : Controllers::AbyssInteractor::SpaceAction::None;
    if (act == Controllers::AbyssInteractor::SpaceAction::Descend) {
        // 重置当层状态并根据 TMX 对象层生成
        if (_monsters) { _monsters->resetFloor(); _monsters->generateInitialWave(); }
        if (_mining) { _mining->generateNodesForFloor(); }
        // 新楼层出生点：优先 Appear，否则楼梯中心
        _player->setPosition(_map->floorSpawnPos());
        if (_uiController) _uiController->setMineFloorNumber(_map->currentFloor());
        // UI 提示：显示当前楼层
        if (_uiController && _player) {
            _uiController->popTextAt(_player->getPosition(), StringUtils::format("Floor %d", _map->currentFloor()), Color3B::YELLOW);
        }
    } else if (act == Controllers::AbyssInteractor::SpaceAction::UseElevator) {
        if (_elevator) _elevator->togglePanel();
    } else if (act == Controllers::AbyssInteractor::SpaceAction::ReturnToFarm) {
        auto farm = FarmScene::create();
        // 在农场场景加载完成后，将出生点设置到 DoorToMine 对象层中心
        farm->setSpawnAtFarmMineDoor();
        auto trans = TransitionFade::create(0.6f, farm);
        Director::getInstance()->replaceScene(trans);
    } else if (act == Controllers::AbyssInteractor::SpaceAction::ReturnToEntrance) {
        // 返回入口（零层）
        _map->loadEntrance();
        if (_monsters) _monsters->resetFloor();
        if (_mining) _mining->resetFloor();
        if (_player) _player->setPosition(_map->entranceBackSpawnPos());
        if (_uiController) _uiController->setMineFloorNumber(_map->currentFloor());
        // UI 提示：返回入口
        if (_uiController && _player) {
            _uiController->popTextAt(_player->getPosition(), "Returned to Entrance", Color3B::YELLOW);
        }
    }
    _inTransition = false;
}

const char* AbyssMineScene::doorPromptText() const { return "Press Space to Descend"; }

void AbyssMineScene::onMouseDown(EventMouse* e) {
    if (_combat) _combat->onMouseDown(e);
}

void AbyssMineScene::onKeyPressedHook(EventKeyboard::KeyCode code) {
    if (code == EventKeyboard::KeyCode::KEY_N) {
        // 调试：直接去下一层
        _map->descend(1);
        if (_monsters) { _monsters->resetFloor(); _monsters->generateInitialWave(); }
        if (_mining) { _mining->generateNodesForFloor(); }
        if (_player) _player->setPosition(_map->floorSpawnPos());
        if (_uiController) _uiController->setMineFloorNumber(_map->currentFloor());
        if (_uiController && _player) {
            _uiController->popTextAt(_player->getPosition(), StringUtils::format("Floor %d", _map->currentFloor()), Color3B::YELLOW);
        }
    }
}
