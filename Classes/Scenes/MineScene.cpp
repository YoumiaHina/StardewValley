#include "Scenes/MineScene.h"
#include "cocos2d.h"
#include "Controllers/Managers/AudioManager.h"
#include "Scenes/FarmScene.h"
#include "Game/Tool/ToolFactory.h"
#include "Game/WorldState.h"
#include "Controllers/Interact/ChestInteractor.h"

USING_NS_CC;

Scene* MineScene::createScene() { return MineScene::create(); }

bool MineScene::init() {
    // 深渊矿洞：不启用空格工具，启用左键工具；不显示 Craft 面板
    if (!initBase(/*worldScale*/3.0f, /*buildCraftPanel*/false, /*enableToolOnSpace*/false, /*enableToolOnLeftClick*/true)) return false;
    auto& ws = Game::globalState();
    ws.lastScene = static_cast<int>(Game::SceneKind::Mine);
    Managers::AudioManager::getInstance().playBackgroundFor(Managers::SceneZone::Abyss);

    // 组合矿洞模块
    _map = static_cast<Controllers::MineMapController*>(_mapController);
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
                        if (_player && _mapController) _uiController->popTextAt(_mapController->getPlayerPosition(_player->getPosition()), "Got Sword!", cocos2d::Color3B::GREEN);
                        break;
                    }
                }
                if (!inserted && _player && _uiController) {
                    if (_mapController) _uiController->popTextAt(_mapController->getPlayerPosition(_player->getPosition()), "Inventory Full", cocos2d::Color3B::RED);
                }
            }
            // 仅在成功放入或已拥有剑时标记为已赠送，避免“未实际获得却阻断后续赠送”
            if (inserted || hasSword) {
                ws.grantedSwordAtEntrance = true;
            }
        }
    }
    _monsters = new Controllers::MineMonsterController(_map, _worldNode, [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); });
    if (_map->currentFloor() > 0) {
        _monsters->generateInitialWave();
    }
    _combat = new Controllers::MineCombatController(_map,
                                                    _monsters,
                                                    [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); },
                                                    [this]() -> Vec2 { return _playerController ? _playerController->lastDir() : Vec2(0,-1); });
    _interactor = new Controllers::MineInteractor(_map, [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); });
    _elevator = new Controllers::MineElevatorController(_map, _monsters, _uiController);
    _elevator->buildPanel();
    _elevator->setOnFloorChanged([this](int floor){
        if (_uiController) _uiController->setMineFloorNumber(floor);
        if (_uiController) _uiController->refreshHotbar();
        if (_player) _player->setPosition(_map->floorSpawnPos());
    });
    // 矿洞 HUD：在能量条正上方构建血条（红色）
    if (_uiController) {
        _uiController->buildHPBarAboveEnergy();
        _uiController->buildMineFloorLabel();
        _uiController->setMineFloorNumber(_map->currentFloor());
    }

    addUpdateCallback([this](float dt){ if (_monsters) _monsters->update(dt); });
    addUpdateCallback([this](float dt){ if (_combat) _combat->update(dt); });
    addUpdateCallback([this](float){ if (_uiController) _uiController->refreshHUD(); });
    addUpdateCallback([this](float){
        auto& ws = Game::globalState();
        if (_map) {
            ws.lastMineFloor = _map->currentFloor();
        }
    });

    return true;
}

MineScene::~MineScene() {
    delete _chestInteractor;
    _chestInteractor = nullptr;
    delete _elevator;
    _elevator = nullptr;
    delete _interactor;
    _interactor = nullptr;
    delete _combat;
    _combat = nullptr;
    delete _monsters;
    _monsters = nullptr;
}

Controllers::IMapController* MineScene::createMapController(Node* worldNode) {
    _map = new Controllers::MineMapController(worldNode);
    return _map;
}

void MineScene::positionPlayerInitial() {
    if (!_map || !_player) return;
    auto& ws = Game::globalState();
    if (ws.lastScene == static_cast<int>(Game::SceneKind::Mine) &&
        (ws.lastPlayerX != 0.0f || ws.lastPlayerY != 0.0f)) {
        _player->setPosition(Vec2(ws.lastPlayerX, ws.lastPlayerY));
        return;
    }
    auto size = _map->getContentSize();
    _player->setPosition(Vec2(size.width * 0.5f, size.height * 0.65f));
}

void MineScene::onSpacePressed() {
    if (_inTransition) return;
    _inTransition = true;
    auto& ws = Game::globalState();
    std::size_t prevElevatorCount = ws.abyssElevatorFloors.size();
    auto act = _interactor ? _interactor->onSpacePressed() : Controllers::MineInteractor::SpaceAction::None;
    if (act == Controllers::MineInteractor::SpaceAction::Descend) {
        bool unlockedNewElevatorFloor = ws.abyssElevatorFloors.size() > prevElevatorCount;
        // 重置当层状态并根据 TMX 对象层生成
        if (_monsters) { _monsters->resetFloor(); _monsters->generateInitialWave(); }
        // 新楼层出生点：优先 Appear，否则楼梯中心
        _player->setPosition(_map->floorSpawnPos());
        if (_uiController) _uiController->setMineFloorNumber(_map->currentFloor());
        if (_uiController) _uiController->refreshHotbar();
        // UI 提示：显示当前楼层
        if (_uiController && _player) {
            if (_mapController) _uiController->popTextAt(_mapController->getPlayerPosition(_player->getPosition()), StringUtils::format("Floor %d", _map->currentFloor()), Color3B::YELLOW);
        }
        if (_uiController && unlockedNewElevatorFloor) {
            _uiController->popCenterBigText("You get a new sword!!!", Color3B::GREEN);
        }
    } else if (act == Controllers::MineInteractor::SpaceAction::UseElevator) {
        if (_elevator) _elevator->togglePanel();
    } else if (act == Controllers::MineInteractor::SpaceAction::ReturnToFarm) {
        auto farm = FarmScene::create();
        // 在农场场景加载完成后，将出生点设置到 DoorToMine 对象层中心
        farm->setSpawnAtFarmMineDoor();
        auto trans = TransitionFade::create(0.6f, farm);
        Director::getInstance()->replaceScene(trans);
    } else if (act == Controllers::MineInteractor::SpaceAction::ReturnToEntrance) {
        // 返回入口（零层）
        _map->loadEntrance();
        if (_monsters) _monsters->resetFloor();
        if (_player) _player->setPosition(_map->entranceBackSpawnPos());
        if (_uiController) _uiController->setMineFloorNumber(_map->currentFloor());
        // UI 提示：返回入口
        if (_uiController && _player) {
            if (_mapController) _uiController->popTextAt(_mapController->getPlayerPosition(_player->getPosition()), "Returned to Entrance", Color3B::YELLOW);
        }
    }
    _inTransition = false;
}

const char* MineScene::doorPromptText() const { return "Press Space to Descend"; }

void MineScene::onMouseDown(EventMouse* e) {
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
        if (!_chestInteractor && _inventory && _mapController && _uiController) {
            _chestInteractor = new Controllers::ChestInteractor(
                _inventory,
                _mapController,
                _uiController,
                [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); },
                [this]() -> Vec2 { return _playerController ? _playerController->lastDir() : Vec2(0,-1); });
        }
        if (_chestInteractor) {
            _chestInteractor->onLeftClick();
        }
        if (_uiController && _uiController->isChestPanelVisible()) {
            return;
        }
        if (_inventory && _inventory->selectedKind() == Game::SlotKind::Item) {
            const auto& slot = _inventory->selectedSlot();
            if (slot.itemType == Game::ItemType::Chest) {
                return;
            }
        }
    }
    if (_combat) _combat->onMouseDown(e);
}

bool MineScene::isMovementBlockedByScene() const {
    return _elevator && _elevator->isPanelVisible();
}

void MineScene::onKeyPressedHook(EventKeyboard::KeyCode code) {
    if (code == EventKeyboard::KeyCode::KEY_N) {
        auto& ws = Game::globalState();
        std::size_t prevElevatorCount = ws.abyssElevatorFloors.size();
        _map->descend(1);
        bool unlockedNewElevatorFloor = ws.abyssElevatorFloors.size() > prevElevatorCount;
        if (_monsters) { _monsters->resetFloor(); _monsters->generateInitialWave(); }
        if (_player) _player->setPosition(_map->floorSpawnPos());
        if (_uiController) _uiController->setMineFloorNumber(_map->currentFloor());
        if (_uiController) _uiController->refreshHotbar();
        if (_uiController && _player) {
            if (_mapController) _uiController->popTextAt(_mapController->getPlayerPosition(_player->getPosition()), StringUtils::format("Floor %d", _map->currentFloor()), Color3B::YELLOW);
            if (unlockedNewElevatorFloor) {
                _uiController->popCenterBigText("You get a new sword!!!", Color3B::GREEN);
            }
        }
    }
}
