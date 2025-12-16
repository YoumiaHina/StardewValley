#include "Scenes/SceneBase.h"
#include "cocos2d.h"
#include "Game/Tool/ToolFactory.h"
#include <string>
#include "Scenes/RoomScene.h"
#include "Game/Chest.h"
#include "Controllers/Systems/FishingController.h"
#include "Game/Tool/FishingRod.h"

using namespace cocos2d;

bool SceneBase::initBase(float worldScale, bool buildCraftPanel, bool enableToolOnSpace, bool enableToolOnLeftClick) {
    if (!Scene::init()) return false;

    // 世界容器
    _worldNode = Node::create();
    _worldNode->setScale(worldScale);
    this->addChild(_worldNode, 0);

    // 地图控制器由子类提供
    _mapController = createMapController(_worldNode);
    if (!_mapController) return false;

    // 角色外观
    auto pv = Game::PlayerView::create();
    _player = pv;
    auto def = UserDefault::getInstance();
    int shirt = def->getIntegerForKey("player_shirt", 0);
    int hair  = def->getIntegerForKey("player_hair", 0);
    int r = def->getIntegerForKey("player_hair_r", 255);
    int g = def->getIntegerForKey("player_hair_g", 255);
    int b = def->getIntegerForKey("player_hair_b", 255);
    int maxShirt = Game::PlayerView::getMaxShirtStyles();
    if (shirt < 0 || shirt >= maxShirt) shirt = 0;
    pv->setShirtStyle(shirt);
    pv->setPantsStyle(0);
    pv->setHairStyle(hair);
    pv->setHairColor(Color3B(r, g, b));
    positionPlayerInitial();

    // 由 MapController 负责选择正确父节点与层级（Farm: TMX，Room: world）。
    _mapController->addActorToMap(_player, 20);

    // 共享背包
    auto &ws = Game::globalState();
    if (!ws.inventory) {
        ws.inventory = std::make_shared<Game::Inventory>(GameConfig::TOOLBAR_SLOTS);
        ws.inventory->setTool(0, Game::makeTool(Game::ToolKind::Axe));
        ws.inventory->setTool(1, Game::makeTool(Game::ToolKind::Hoe));
        ws.inventory->setTool(2, Game::makeTool(Game::ToolKind::Pickaxe));
        ws.inventory->setTool(3, Game::makeTool(Game::ToolKind::WaterCan));
        ws.inventory->setTool(4, Game::makeTool(Game::ToolKind::FishingRod));
    }
    _inventory = ws.inventory;
    if (_inventory) _inventory->selectIndex(ws.selectedIndex);

    // UI 控制器
    _uiController = new Controllers::UIController(this, _worldNode, _inventory);
    _uiController->buildHUD();
    _uiController->setInventoryBackground("inventory.png");
    _uiController->buildHotbar();
    _uiController->buildChestPanel();
    if (buildCraftPanel) _uiController->buildCraftPanel();

    // 玩家/时间控制器
    _playerController = new Controllers::PlayerController(_player, _mapController, _worldNode);
    _cropSystem = new Controllers::CropSystem();
    _stateController = new Controllers::GameStateController(_mapController, _uiController, _cropSystem);

    _fishingController = new Controllers::FishingController(_mapController, _inventory, _uiController, this, _worldNode);
    addUpdateCallback([this](float dt) {
        if (_fishingController) _fishingController->update(dt);
    });
    if (_inventory && _fishingController) {
        for (std::size_t i = 0; i < _inventory->size(); ++i) {
            auto tb = _inventory->toolAtMutable(i);
            if (tb && tb->kind() == Game::ToolKind::FishingRod) {
                auto rod = dynamic_cast<Game::FishingRod*>(tb);
                if (rod) {
                    rod->setFishingStarter(
                        [this](const Vec2& pos) {
                            if (_fishingController) _fishingController->startAt(pos);
                        });
                }
                break;
            }
        }
    }
    if (_fishingController && _playerController) {
        _fishingController->setMovementLocker(
            [this](bool locked) {
                if (_playerController) _playerController->setMovementLocked(locked);
            });
    }

    // 事件监听
    registerCommonInputHandlers(enableToolOnSpace, enableToolOnLeftClick, buildCraftPanel);
    this->scheduleUpdate();
    return true;
}

void SceneBase::registerCommonInputHandlers(bool enableToolOnSpace, bool enableToolOnLeftClick, bool buildCraftPanel) {
    // 键盘
    auto kb = EventListenerKeyboard::create();
    kb->onKeyPressed = [this, enableToolOnSpace](EventKeyboard::KeyCode code, Event*) {
        bool chestOpen = _uiController && _uiController->isChestPanelVisible();
        bool storeOpen = _uiController && (_uiController->isStorePanelVisible() || _uiController->isAnimalStorePanelVisible());
        if (_uiController && _uiController->isNpcSocialVisible()) {
            return;
        }
        _playerController->onKeyPressed(code);
        switch (code) {
            case EventKeyboard::KeyCode::KEY_1: _uiController->selectHotbarIndex(0); break;
            case EventKeyboard::KeyCode::KEY_2: _uiController->selectHotbarIndex(1); break;
            case EventKeyboard::KeyCode::KEY_3: _uiController->selectHotbarIndex(2); break;
            case EventKeyboard::KeyCode::KEY_4: _uiController->selectHotbarIndex(3); break;
            case EventKeyboard::KeyCode::KEY_5: _uiController->selectHotbarIndex(4); break;
            case EventKeyboard::KeyCode::KEY_6: _uiController->selectHotbarIndex(5); break;
            case EventKeyboard::KeyCode::KEY_7: _uiController->selectHotbarIndex(6); break;
            case EventKeyboard::KeyCode::KEY_8: _uiController->selectHotbarIndex(7); break;
            case EventKeyboard::KeyCode::KEY_9: _uiController->selectHotbarIndex(8); break;
            case EventKeyboard::KeyCode::KEY_0: _uiController->selectHotbarIndex(9); break;
            case EventKeyboard::KeyCode::KEY_Z: {
                if (chestOpen || storeOpen) break;
                Game::Cheat::grantBasic(_inventory);
                _uiController->refreshHotbar();
            } break;
            case EventKeyboard::KeyCode::KEY_F6: {
                if (chestOpen || storeOpen) break;
                Game::Cheat::grantProduce(_inventory, Game::CropType::Eggplant, 5);
                _uiController->refreshHotbar();
                if (_player) _uiController->popTextAt(_player->getPosition(), "Eggplant x5", Color3B::YELLOW);
            } break;
            case EventKeyboard::KeyCode::KEY_E: {
                if (chestOpen || storeOpen) break;
                Game::openGlobalChest(_uiController);
            } break;
            case EventKeyboard::KeyCode::KEY_X: {
                if (chestOpen || storeOpen) break;
                // 仅让当前目标格子的作物提升一个阶段
                int tc = 0, tr = 0;
                if (_player && _mapController) {
                    Vec2 playerPos = _player->getPosition();
                    Vec2 dir = _playerController ? _playerController->lastDir() : Vec2(0,-1);
                    auto tgt = _mapController->targetTile(playerPos, dir);
                    tc = tgt.first; tr = tgt.second;
                    if (_cropSystem) { _cropSystem->advanceCropOnceAt(tc, tr); }
                }
                if (_uiController && _player) {
                    _mapController->refreshCropsVisuals();
                    _uiController->popTextAt(_player->getPosition(), "Grow +1", Color3B::YELLOW);
                }
            } break;
            case EventKeyboard::KeyCode::KEY_F: {
                if (chestOpen || storeOpen) break;
                auto &ws = Game::globalState();
                if (_inventory && _inventory->selectedKind() == Game::SlotKind::Item) {
                    auto slot = _inventory->selectedSlot();
                    if (slot.itemQty > 0 && Game::itemEdible(slot.itemType)) {
                        bool ok = _inventory->consumeSelectedItem(1);
                        if (ok) {
                            int e = ws.energy + Game::itemEnergyRestore(slot.itemType);
                            int h = ws.hp + Game::itemHpRestore(slot.itemType);
                            ws.energy = std::min(ws.maxEnergy, e);
                            ws.hp = std::min(ws.maxHp, h);
                            _uiController->refreshHotbar();
                            _uiController->refreshHUD();
                            if (_player) _uiController->popTextAt(_player->getPosition(), "Ate", Color3B::GREEN);
                        }
                    }
                }
            } break;
            case EventKeyboard::KeyCode::KEY_SPACE: {
                if (chestOpen || storeOpen) break;
                if (enableToolOnSpace) {
                    bool nearDoor = false;
                    if (_player && _mapController) {
                        Vec2 p = _player->getPosition();
                        nearDoor = _mapController->isNearDoor(p)
                                   || _mapController->isNearMineDoor(p)
                                   || _mapController->isNearBeachDoor(p)
                                   || _mapController->isNearFarmDoor(p)
                                   || _mapController->isNearTownDoor(p);
                    }
                    if (!nearDoor) {
                        auto t = _inventory ? _inventory->selectedTool() : nullptr;
                        if (t) {
                            std::string msg = t->use(_mapController, _cropSystem,
                                [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); },
                                [this]() -> Vec2 { return _playerController ? _playerController->lastDir() : Vec2(0,-1); },
                                _uiController);
                            if (_player && !msg.empty()) {
                                _player->playToolAnimation(t->kind());
                            }
                        }
                    }
                }
                onSpacePressed();
            } break;
            default: break;
        }
        // 子类扩展键盘处理（如快速进入矿洞 K 键）
        onKeyPressedHook(code);
    };
    kb->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) {
        _playerController->onKeyReleased(code);
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(kb, this);

    // 鼠标
    auto mouse = EventListenerMouse::create();
    mouse->onMouseDown = [this, enableToolOnLeftClick](EventMouse* e){
        bool chestOpen = _uiController && _uiController->isChestPanelVisible();
        bool storeOpen = _uiController && (_uiController->isStorePanelVisible() || _uiController->isAnimalStorePanelVisible());
        if (_uiController && _uiController->isNpcSocialVisible()) {
            return;
        }
        if (_uiController && _uiController->handleHotbarMouseDown(e)) return;
        if (chestOpen || storeOpen) return;
        if (_mapController && _player) {
            Vec2 clickScene = e->getLocation();
            auto parent = dynamic_cast<Node*>(_player)->getParent();
            Vec2 mapPos = parent ? parent->convertToNodeSpace(clickScene) : clickScene;
            _mapController->setLastClickWorldPos(mapPos);
        }
        if (enableToolOnLeftClick && e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            auto t = _inventory ? _inventory->selectedTool() : nullptr;
            if (t) {
                std::string msg = t->use(_mapController, _cropSystem,
                    [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); },
                    [this]() -> Vec2 { return _playerController ? _playerController->lastDir() : Vec2(0,-1); },
                    _uiController);
                if (_player && !msg.empty()) {
                    _player->playToolAnimation(t->kind());
                }
            }
        }
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            _uiController->handleChestRightClick(e, _mapController->chests());
        }
        onMouseDown(e);
        if (_mapController) {
            _mapController->clearLastClickWorldPos();
        }
    };
    mouse->onMouseScroll = [this](EventMouse* e){
        _uiController->handleHotbarScroll(e->getScrollY());
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);

    // 触摸
    auto touch = EventListenerTouchOneByOne::create();
    touch->onTouchBegan = [this](Touch* t, Event*){
        return _uiController->handleHotbarAtPoint(t->getLocation());
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touch, this);
}

void SceneBase::update(float dt) {
    auto& ws = Game::globalState();
    if (ws.hp <= 0) {
        ws.gold = ws.gold > 0 ? ws.gold / 2 : 0;
        ws.hp = ws.maxHp;
        auto room = RoomScene::create();
        auto trans = TransitionFade::create(0.6f, room);
        Director::getInstance()->replaceScene(trans);
        return;
    }
    _stateController->update(dt);
    bool blockMove = _uiController && (_uiController->isNpcSocialVisible()
                                       || _uiController->isChestPanelVisible()
                                       || _uiController->isStorePanelVisible()
                                       || _uiController->isAnimalStorePanelVisible());
    if (!blockMove) {
        _playerController->update(dt);
    }
    for (auto& cb : _extraUpdates) { cb(dt); }
    if (_player && _uiController && _mapController) {
        Vec2 p = _player->getPosition();
        if (_inventory) {
            _mapController->collectDropsNear(p, _inventory.get());
        }
        bool nearDoor = _mapController->isNearDoor(p);
        _uiController->showDoorPrompt(nearDoor, p, doorPromptText());
        bool nearLake = _mapController->isNearLake(p, _mapController->tileSize() * (GameConfig::LAKE_REFILL_RADIUS_TILES + 0.5f));
        bool rodSelected = (_inventory && _inventory->selectedTool() && _inventory->selectedTool()->kind() == Game::ToolKind::FishingRod);
        bool canShowFishPrompt = nearLake && rodSelected && !ws.fishingActive;
        cocos2d::Vec2 fishPos = p;
        auto parent = _player->getParent();
        if (parent && _worldNode) {
            cocos2d::Vec2 world = parent->convertToWorldSpace(p);
            fishPos = _worldNode->convertToNodeSpace(world);
        }
        _uiController->showFishPrompt(canShowFishPrompt, fishPos, "Space/Left-click to Fish");
    }
}

void SceneBase::addUpdateCallback(const std::function<void(float)>& cb) {
    _extraUpdates.push_back(cb);
}
#include "Game/Tool/ToolFactory.h"
