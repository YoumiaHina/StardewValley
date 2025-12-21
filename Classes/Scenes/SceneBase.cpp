#include "Scenes/SceneBase.h"
#include "cocos2d.h"
#include "Game/Tool/ToolFactory.h"
#include <algorithm>
#include <string>
#include "Scenes/RoomScene.h"
#include "Game/Chest.h"
#include "Game/Map/RoomMap.h"
#include "Controllers/Systems/FishingController.h"
#include "Controllers/Systems/WeatherController.h"
#include "Controllers/Systems/FestivalController.h"
#include "Game/Tool/FishingRod.h"

using namespace cocos2d;

namespace {

cocos2d::Vec2 computeRoomBedCenter() {
    auto* roomMap = Game::RoomMap::create("Maps/farm_room/farm_room.tmx");
    if (roomMap) {
        const auto& beds = roomMap->bedRects();
        cocos2d::Rect bedRect;
        if (!beds.empty()) {
            float minX = beds[0].getMinX(), minY = beds[0].getMinY();
            float maxX = beds[0].getMaxX(), maxY = beds[0].getMaxY();
            for (std::size_t i = 1; i < beds.size(); ++i) {
                minX = std::min(minX, beds[i].getMinX());
                minY = std::min(minY, beds[i].getMinY());
                maxX = std::max(maxX, beds[i].getMaxX());
                maxY = std::max(maxY, beds[i].getMaxY());
            }
            bedRect = cocos2d::Rect(minX, minY, maxX - minX, maxY - minY);
        } else {
            cocos2d::Size cs = roomMap->getContentSize();
            cocos2d::Rect roomRect(0, 0, cs.width, cs.height);
            float bedW = 120.0f;
            float bedH = 60.0f;
            bedRect = cocos2d::Rect(roomRect.getMinX() + 24.0f,
                                    roomRect.getMaxY() - bedH - 24.0f,
                                    bedW, bedH);
        }
        return cocos2d::Vec2(bedRect.getMidX(), bedRect.getMidY());
    }
    return cocos2d::Vec2(60.0f, 60.0f);
}

} // namespace

bool SceneBase::initBase(float worldScale, bool buildCraftPanel, bool enableToolOnSpace, bool enableToolOnLeftClick) {
    if (!Scene::init()) return false;

    // 世界容器
    _worldNode = Node::create();
    _worldNode->setScale(worldScale);
    this->addChild(_worldNode, 0);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _dayNightOverlay = LayerColor::create(Color4B(0, 0, 0, 0), visibleSize.width, visibleSize.height);
    if (_dayNightOverlay) {
        _dayNightOverlay->setPosition(origin);
        this->addChild(_dayNightOverlay, 1);
    }

    // 地图控制器由子类提供
    _mapController = createMapController(_worldNode);
    if (!_mapController) return false;

    // 角色外观
    auto pv = Game::PlayerView::create();
    _player = pv;
    auto &ws = Game::globalState();
    int shirt = ws.playerShirt;
    int pants = ws.playerPants;
    int hair  = ws.playerHair;
    int r = ws.playerHairR;
    int g = ws.playerHairG;
    int b = ws.playerHairB;
    int maxShirt = Game::PlayerView::getMaxShirtStyles();
    int maxPants = Game::PlayerView::getMaxPantsStyles();
    int maxHair = Game::PlayerView::getMaxHairStyles();
    if (shirt < 0 || shirt >= maxShirt) shirt = 0;
    if (pants < 0 || pants >= maxPants) pants = 0;
    if (hair < 0 || hair >= maxHair) hair = 0;
    pv->setShirtStyle(shirt);
    pv->setPantsStyle(pants);
    pv->setHairStyle(hair);
    pv->setHairColor(Color3B(r, g, b));
    positionPlayerInitial();
    pv->syncUpperLayerPosition();

    // 由 MapController 负责选择正确父节点与层级（Farm: TMX，Room: world）。
    _mapController->addActorToMap(_player, 20);
    if (_mapController) {
        auto* upper = pv->upperNode();
        if (upper) {
            _mapController->addActorToOverlay(upper, 23);
        }
    }

    // 共享背包
    if (!ws.inventory) {
        ws.inventory = std::make_shared<Game::Inventory>(GameConfig::TOOLBAR_SLOTS);
        ws.inventory->setTool(0, Game::makeTool(Game::ToolKind::Axe));
        ws.inventory->setTool(1, Game::makeTool(Game::ToolKind::Hoe));
        ws.inventory->setTool(2, Game::makeTool(Game::ToolKind::Pickaxe));
        ws.inventory->setTool(3, Game::makeTool(Game::ToolKind::WaterCan));
        ws.inventory->setTool(4, Game::makeTool(Game::ToolKind::FishingRod));
        ws.inventory->setTool(5, Game::makeTool(Game::ToolKind::Scythe));
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

    if (_mapController && _mapController->supportsWeather()) {
        _weatherController = new Controllers::WeatherController(_mapController, _worldNode, _playerController);
        addUpdateCallback([this](float dt) {
            if (_weatherController) _weatherController->update(dt);
        });
    }

    if (_mapController) {
        _festivalController = new Controllers::FestivalController(_mapController);
        addUpdateCallback([this](float dt) {
            if (_festivalController) _festivalController->update(dt);
        });
    }

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
    if (_player) {
        Vec2 p = _player->getPosition();
        ws.lastPlayerX = p.x;
        ws.lastPlayerY = p.y;
    }
    if (ws.hp <= 0 || ws.pendingPassOut) {
        ws.pendingPassOut = false;
        cocos2d::Vec2 bed = computeRoomBedCenter();
        ws.lastScene = static_cast<int>(Game::SceneKind::Room);
        ws.lastPlayerX = bed.x;
        ws.lastPlayerY = bed.y;
        ws.gold = ws.gold > 0 ? ws.gold / 2 : 0;
        ws.hp = ws.maxHp;
        auto room = RoomScene::create();
        auto trans = TransitionFade::create(0.6f, room);
        Director::getInstance()->replaceScene(trans);
        if (_stateController) {
            _stateController->sleepToNextMorning();
        }
        return;
    }
    _stateController->update(dt);
    if (_dayNightOverlay) {
        if (_mapController && _mapController->supportsWeather()) {
            int minutes = ws.timeHour * 60 + ws.timeMinute;
            const int start = 17 * 60 + 30;
            const int end = 19 * 60;
            const int maxOpacity = 120;
            int opacity = 0;
            if (minutes >= end) {
                opacity = maxOpacity;
            } else if (minutes > start) {
                float t = static_cast<float>(minutes - start) / static_cast<float>(end - start);
                opacity = static_cast<int>(static_cast<float>(maxOpacity) * t);
            }
            _dayNightOverlay->setOpacity(static_cast<GLubyte>(std::max(0, std::min(255, opacity))));
            _dayNightOverlay->setVisible(true);
        } else {
            _dayNightOverlay->setOpacity(0);
            _dayNightOverlay->setVisible(false);
        }
    }
    bool blockMove = ws.fishingActive
                     || (_uiController && (_uiController->isDialogueVisible()
                                           || _uiController->isNpcSocialVisible()
                                           || _uiController->isChestPanelVisible()
                                           || _uiController->isStorePanelVisible()
                                           || _uiController->isAnimalStorePanelVisible()
                                           || _uiController->isToolUpgradePanelVisible()));
    if (_playerController) {
        _playerController->setMovementLocked(blockMove);
        _playerController->update(dt);
    }
    for (auto& cb : _extraUpdates) { cb(dt); }
    if (_player && _uiController && _mapController) {
        Vec2 p = _player->getPosition();
        if (_inventory) {
            _mapController->collectDropsNear(p, _inventory.get());
            _uiController->refreshHotbar();
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
