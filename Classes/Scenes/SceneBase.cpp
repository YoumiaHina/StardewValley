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
    (void)buildCraftPanel;
    if (!_playerController) return;
    _playerController->registerCommonInputHandlers(
        this,
        _uiController,
        _inventory,
        _cropSystem,
        enableToolOnSpace,
        enableToolOnLeftClick,
        [this]() { onSpacePressed(); },
        [this](EventKeyboard::KeyCode code) { onKeyPressedHook(code); },
        [this](EventMouse* e) { onMouseDown(e); });
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
    bool blockMove = _uiController && (_uiController->isDialogueVisible()
                                       || _uiController->isNpcSocialVisible()
                                       || _uiController->isChestPanelVisible()
                                       || _uiController->isStorePanelVisible()
                                       || _uiController->isAnimalStorePanelVisible());
    if (_playerController) {
        _playerController->setMovementLocked(blockMove);
        _playerController->update(dt);
    }
    for (auto& cb : _extraUpdates) { cb(dt); }
    if (_player && _uiController && _mapController) {
        Vec2 p = _player->getPosition();
        if (_inventory) {
            _mapController->collectDropsNear(p, _inventory.get());
        }
        bool nearDoor = _mapController->isNearDoor(p);
        _uiController->showDoorPrompt(nearDoor, _mapController->getPlayerPosition(p), doorPromptText());
        bool nearLake = _mapController->isNearLake(p, _mapController->tileSize() * (GameConfig::LAKE_REFILL_RADIUS_TILES + 0.5f));
        bool rodSelected = (_inventory && _inventory->selectedTool() && _inventory->selectedTool()->kind() == Game::ToolKind::FishingRod);
        bool canShowFishPrompt = nearLake && rodSelected && !ws.fishingActive;
        _uiController->showFishPrompt(canShowFishPrompt, _mapController->getPlayerPosition(p), "Left-click to Fish");
    }
}

void SceneBase::addUpdateCallback(const std::function<void(float)>& cb) {
    _extraUpdates.push_back(cb);
}
