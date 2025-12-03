/**
 * GameScene: 精简场景，仅负责：创建场景、加载模块、调度更新、分发输入事件、控制场景切换。
 */
#include "Scenes/GameScene.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Game/Inventory.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Tool.h"
#include "Scenes/RoomScene.h"
#include "Controllers/PlayerController.h"
#include "Controllers/FarmMapController.h"
#include "Controllers/UIController.h"
#include "Controllers/ToolSystem.h"
#include "Controllers/GameStateController.h"
#include "Controllers/FarmInteractor.h"

USING_NS_CC;

Scene* GameScene::createScene() {
	return GameScene::create();
}

bool GameScene::init() {
	if (!Scene::init()) return false;

	// 世界容器
	_worldNode = Node::create();
	_worldNode->setScale(3.0f);
	this->addChild(_worldNode, 0);

	// 地图控制器
	_mapController = new Controllers::FarmMapController(_worldNode);
	_mapController->init();

	// 角色外观节点
	_player = Game::PlayerAppearance::create();
	// 从用户配置恢复外观
	auto def = UserDefault::getInstance();
	int shirt = def->getIntegerForKey("player_shirt", 0);
	int pants = def->getIntegerForKey("player_pants", 0);
	int hair = def->getIntegerForKey("player_hair", 0);
	int r = def->getIntegerForKey("player_hair_r", 255);
	int g = def->getIntegerForKey("player_hair_g", 255);
	int b = def->getIntegerForKey("player_hair_b", 255);
	_player->setShirtStyle(shirt);
	_player->setPantsStyle(pants);
	_player->setHairStyle(hair);
	_player->setHairColor(Color3B(r, g, b));
	// 初始放在中心附近
	_player->setPosition(_mapController->tileToWorld(GameConfig::MAP_COLS / 2, GameConfig::MAP_ROWS / 2));
	if (auto tmx = _mapController->tmx()){
		tmx->getTMX()->addChild(_player, 20);
	}
	else{
		_worldNode->addChild(_player, 2);
	}

	// 共享背包
	auto& ws = Game::globalState();
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
	_uiController->buildCraftPanel();

	// 玩家控制器
	_playerController = new Controllers::PlayerController(_player, _mapController, _worldNode);

	// 工具系统
	_toolSystem = new Controllers::ToolSystem(_inventory, _mapController, [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); }, _uiController);

	// 时间控制器
	_stateController = new Controllers::GameStateController(_mapController, _uiController);

	// 交互控制器
	_interactor = new Controllers::FarmInteractor(_inventory, _mapController, _uiController, [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); });

	// 事件监听（全部转发到控制器）
	auto kb = EventListenerKeyboard::create();
	kb->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*) {
		_playerController->onKeyPressed(code);
		// 热键栏数字键选择
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
				if (act == Controllers::FarmInteractor::SpaceAction::EnterHouse) {
					auto room = RoomScene::create();
					room->setSpawnInsideDoor();
					auto trans = TransitionFade::create(0.6f, room);
					Director::getInstance()->replaceScene(trans);
				}
			} break;
			default: break;
		}
		};
	kb->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) {
		_playerController->onKeyReleased(code);
		};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(kb, this);

	auto mouse = EventListenerMouse::create();
	mouse->onMouseDown = [this](EventMouse* e) {
		// 左键选择热键栏；右键靠近箱子时打开面板
		if (_uiController->handleHotbarMouseDown(e)) return;
		if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
			auto& chs = static_cast<Controllers::FarmMapController*>(_mapController)->chests();
			_uiController->handleChestRightClick(e, chs);
		}
		};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);

	auto touch = EventListenerTouchOneByOne::create();
	touch->onTouchBegan = [this](Touch* t, Event*) {
		// 触摸热键栏选择（移动端）
		return _uiController->handleHotbarAtPoint(t->getLocation());
		};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touch, this);

	this->scheduleUpdate();
	return true;
}

void GameScene::update(float dt) {
	// 调度：先时间推进，再玩家移动，最后提示更新
	_stateController->update(dt);
	_playerController->update(dt);

	// 提示：门与箱子（由 MapController 检测，UIController 显示）
	if (_player && _uiController && _mapController) {
		Vec2 p = _player->getPosition();
		bool nearDoor = _mapController->isNearDoor(p);
		bool nearChest = _mapController->isNearChest(p);
		_uiController->showDoorPrompt(nearDoor, p, "Press Space to Enter House");
		_uiController->showChestPrompt(nearChest, p, "Right-click to Open / Space to Deposit");
	}
}

void GameScene::setSpawnAtFarmEntrance() {
	if (!_player || !_mapController) return;
	float s = static_cast<float>(GameConfig::TILE_SIZE);
	// 使用门矩形中点向上偏移 1 格
	auto& door = static_cast<Controllers::FarmMapController*>(_mapController); // for clarity only
	// 简化：直接取门矩形由 map 内部维护；此处以玩家当前位置替代具体门口位置以避免强耦合
	_player->setPosition(_player->getPosition() + Vec2(0, s));
}