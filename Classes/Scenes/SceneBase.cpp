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
    _player = Game::PlayerAppearance::create();
    auto def = UserDefault::getInstance();
    int shirt = def->getIntegerForKey("player_shirt", 0);
    int pants = def->getIntegerForKey("player_pants", 0);
    int hair  = def->getIntegerForKey("player_hair", 0);
    int r = def->getIntegerForKey("player_hair_r", 255);
    int g = def->getIntegerForKey("player_hair_g", 255);
    int b = def->getIntegerForKey("player_hair_b", 255);
    _player->setShirtStyle(shirt);
    _player->setPantsStyle(pants);
    _player->setHairStyle(hair);
    _player->setHairColor(Color3B(r, g, b));
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
    }
    _inventory = ws.inventory;
    if (_inventory) _inventory->selectIndex(ws.selectedIndex);

    // UI 控制器
    _uiController = new Controllers::UIController(this, _worldNode, _inventory);
    _uiController->buildHUD();
    _uiController->buildHotbar();
    _uiController->buildChestPanel();
    if (buildCraftPanel) _uiController->buildCraftPanel();

    // 玩家/时间控制器
    _playerController = new Controllers::PlayerController(_player, _mapController, _worldNode);
    _stateController = new Controllers::GameStateController(_mapController, _uiController);

    // 可选工具系统
    if (enableToolOnSpace || enableToolOnLeftClick) {
        _toolSystem = new Controllers::ToolSystem(_inventory, _mapController,
            [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); }, _uiController);
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
            case EventKeyboard::KeyCode::KEY_SPACE: {
                if (enableToolOnSpace && _toolSystem) { _toolSystem->useSelectedTool(); }
                onSpacePressed();
            } break;
            default: break;
        }
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
    if (_player && _uiController && _mapController) {
        Vec2 p = _player->getPosition();
        bool nearDoor = _mapController->isNearDoor(p);
        bool nearChest = _mapController->isNearChest(p);
        _uiController->showDoorPrompt(nearDoor, p, doorPromptText());
        _uiController->showChestPrompt(nearChest, p, "Right-click to Open / Space to Deposit");
    }
}