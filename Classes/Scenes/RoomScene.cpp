/**
 * RoomScene: 精简场景，仅负责组合模块、更新调度与事件分发。
 */
#include "Scenes/RoomScene.h"
#include "Scenes/GameScene.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Game/Inventory.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Controllers/PlayerController.h"
#include "Controllers/RoomMapController.h"
#include "Controllers/UIController.h"
#include "Controllers/GameStateController.h"
#include "Controllers/RoomInteractor.h"

USING_NS_CC;

Scene* RoomScene::createScene() {
    return RoomScene::create();
}

bool RoomScene::init() {
    if (!Scene::init()) return false;

    _worldNode = Node::create();
    _worldNode->setScale(2.0f);
    this->addChild(_worldNode, 0);

    _mapController = new Controllers::RoomMapController(_worldNode);
    _mapController->init();

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
    _player->setPosition(Vec2(_mapController->roomRect().getMidX(), _mapController->roomRect().getMidY() + 80));
    _worldNode->addChild(_player, 2);

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

    _uiController = new Controllers::UIController(this, _worldNode, _inventory);
    _uiController->buildHUD();
    _uiController->buildHotbar();
    _uiController->buildChestPanel();

    _playerController = new Controllers::PlayerController(_player, _mapController, _worldNode);
    _stateController = new Controllers::GameStateController(_mapController, _uiController);
    _interactor = new Controllers::RoomInteractor(_inventory, _mapController, _uiController, [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); });

    auto kb = EventListenerKeyboard::create();
    kb->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*){
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
            case EventKeyboard::KeyCode::KEY_SPACE: {
                auto act = _interactor->onSpacePressed();
                if (act == Controllers::RoomInteractor::SpaceAction::ExitHouse) {
                    auto farm = GameScene::create();
                    farm->setSpawnAtFarmEntrance();
                    auto trans = TransitionFade::create(0.6f, farm);
                    Director::getInstance()->replaceScene(trans);
                }
            } break;
            default: break;
        }
    };
    kb->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*){ _playerController->onKeyReleased(code); };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(kb, this);

    auto mouse = EventListenerMouse::create();
    mouse->onMouseDown = [this](EventMouse* e){
        if (_uiController->handleHotbarMouseDown(e)) return;
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            _uiController->handleChestRightClick(e, _mapController->chests());
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);

    auto touch = EventListenerTouchOneByOne::create();
    touch->onTouchBegan = [this](Touch* t, Event*){
        return _uiController->handleHotbarAtPoint(t->getLocation());
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touch, this);

    this->scheduleUpdate();
    return true;
}

void RoomScene::update(float dt) {
    _stateController->update(dt);
    _playerController->update(dt);

    if (_player && _uiController && _mapController) {
        Vec2 p = _player->getPosition();
        bool nearDoor = _mapController->isNearDoor(p);
        bool nearChest = _mapController->isNearChest(p);
        _uiController->showDoorPrompt(nearDoor, p, "Press Space to Exit");
        _uiController->showChestPrompt(nearChest, p, "Right-click to Open / Space to Deposit");
    }
}

void RoomScene::setSpawnInsideDoor() {
    if (!_player || !_mapController) return;
    float offsetY = 28.0f;
    Vec2 spawn(_mapController->doorRect().getMidX(), _mapController->doorRect().getMinY() + _mapController->doorRect().size.height + offsetY);
    _player->setPosition(spawn);
}