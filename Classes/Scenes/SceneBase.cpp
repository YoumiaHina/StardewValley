#include "Scenes/SceneBase.h"
#include "cocos2d.h"

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
    int pants = def->getIntegerForKey("player_pants", 0);
    int hair  = def->getIntegerForKey("player_hair", 0);
    int r = def->getIntegerForKey("player_hair_r", 255);
    int g = def->getIntegerForKey("player_hair_g", 255);
    int b = def->getIntegerForKey("player_hair_b", 255);
    pv->setShirtStyle(shirt);
    pv->setPantsStyle(pants);
    pv->setHairStyle(hair);
    pv->setHairColor(Color3B(r, g, b));
    positionPlayerInitial();

    // 由 MapController 负责选择正确父节点与层级（Farm: TMX，Room: world）。
    _mapController->addActorToMap(_player, 20);

    // 共享背包
    auto &ws = Game::globalState();
    if (!ws.inventory) {
        ws.inventory = std::make_shared<Game::Inventory>(GameConfig::TOOLBAR_SLOTS);
        ws.inventory->setTool(0, Game::makeTool(Game::ToolType::Axe));
        ws.inventory->setTool(1, Game::makeTool(Game::ToolType::Hoe));
        ws.inventory->setTool(2, Game::makeTool(Game::ToolType::Pickaxe));
        ws.inventory->setTool(3, Game::makeTool(Game::ToolType::WateringCan));
        ws.inventory->setTool(4, Game::makeTool(Game::ToolType::FishingRod));
    }
    _inventory = ws.inventory;
    if (_inventory) _inventory->selectIndex(ws.selectedIndex);

    // UI 控制器
    _uiController = new Controllers::UIController(this, _worldNode, _inventory);
    _uiController->buildHUD();
    _uiController->buildHotbar();
    // 在水壶上方构建水量蓝条
    _uiController->buildWaterBarAboveCan();
    // 初次刷新蓝条以显示当前水量
    _uiController->refreshWaterBar();
    _uiController->buildChestPanel();
    if (buildCraftPanel) _uiController->buildCraftPanel();

    // 玩家/时间控制器
    _playerController = new Controllers::PlayerController(_player, _mapController, _worldNode);
    _cropSystem = new Controllers::CropSystem();
    _stateController = new Controllers::GameStateController(_mapController, _uiController, _cropSystem);

    // 可选工具系统
    if (enableToolOnSpace || enableToolOnLeftClick) {
        _toolSystem = new Controllers::ToolSystem(_inventory, _mapController, _cropSystem,
            [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); },
            [this]() -> Vec2 { return _playerController ? _playerController->lastDir() : Vec2(0,-1); },
            _uiController);
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
                Game::Cheat::grantBasic(_inventory);
                _uiController->refreshHotbar();
            } break;
            case EventKeyboard::KeyCode::KEY_F1: {
                Game::Cheat::grantSeed(_inventory, Game::CropType::Parsnip, 10);
                _uiController->refreshHotbar();
                if (_player) _uiController->popTextAt(_player->getPosition(), "Parsnip Seed x10", Color3B::YELLOW);
            } break;
            case EventKeyboard::KeyCode::KEY_F2: {
                Game::Cheat::grantSeed(_inventory, Game::CropType::Blueberry, 10);
                _uiController->refreshHotbar();
                if (_player) _uiController->popTextAt(_player->getPosition(), "Blueberry Seed x10", Color3B::YELLOW);
            } break;
            case EventKeyboard::KeyCode::KEY_F3: {
                Game::Cheat::grantSeed(_inventory, Game::CropType::Eggplant, 10);
                _uiController->refreshHotbar();
                if (_player) _uiController->popTextAt(_player->getPosition(), "Eggplant Seed x10", Color3B::YELLOW);
            } break;
            case EventKeyboard::KeyCode::KEY_F4: {
                Game::Cheat::grantProduce(_inventory, Game::CropType::Parsnip, 5);
                _uiController->refreshHotbar();
                if (_player) _uiController->popTextAt(_player->getPosition(), "Parsnip x5", Color3B::YELLOW);
            } break;
            case EventKeyboard::KeyCode::KEY_F5: {
                Game::Cheat::grantProduce(_inventory, Game::CropType::Blueberry, 5);
                _uiController->refreshHotbar();
                if (_player) _uiController->popTextAt(_player->getPosition(), "Blueberry x5", Color3B::YELLOW);
            } break;
            case EventKeyboard::KeyCode::KEY_F6: {
                Game::Cheat::grantProduce(_inventory, Game::CropType::Eggplant, 5);
                _uiController->refreshHotbar();
                if (_player) _uiController->popTextAt(_player->getPosition(), "Eggplant x5", Color3B::YELLOW);
            } break;
            case EventKeyboard::KeyCode::KEY_X: {
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
                if (enableToolOnSpace && _toolSystem) { _toolSystem->useSelectedTool(); }
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
        if (_uiController->handleHotbarMouseDown(e)) return;
        if (enableToolOnLeftClick && e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            if (_toolSystem) { _toolSystem->useSelectedTool(); }
            return;
        }
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            _uiController->handleChestRightClick(e, _mapController->chests());
        }
        // 允许子类进一步处理（例如战斗攻击）
        onMouseDown(e);
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
    _stateController->update(dt);
    _playerController->update(dt);
    for (auto& cb : _extraUpdates) { cb(dt); }
    if (_player && _uiController && _mapController) {
        Vec2 p = _player->getPosition();
        bool nearDoor = _mapController->isNearDoor(p);
        bool nearChest = _mapController->isNearChest(p);
        _uiController->showDoorPrompt(nearDoor, p, doorPromptText());
        _uiController->showChestPrompt(nearChest, p, "Right-click to Open / Space to Deposit");
        bool nearLake = _mapController->isNearLake(p, _mapController->tileSize() * (GameConfig::LAKE_REFILL_RADIUS_TILES + 0.5f));
        bool rodSelected = (_inventory && _inventory->selectedTool() && _inventory->selectedTool()->type == Game::ToolType::FishingRod);
        _uiController->showFishPrompt(nearLake && rodSelected, p, "Space/Left-click to Fish");
    }
}

void SceneBase::addUpdateCallback(const std::function<void(float)>& cb) {
    _extraUpdates.push_back(cb);
}
