#include "Controllers/Input/PlayerController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/Systems/ChestController.h"
#include "Controllers/UI/UIController.h"
#include "Game/View/IPlayerView.h"
#include "Game/WorldState.h"
#include "Game/Cheat.h"
#include "Game/Inventory.h"
#include "Game/Tool/ToolBase.h"
#include "Game/Drop.h"
#include "Game/Save/SaveSystem.h"
#include "Game/SkillTree/SkillTreeSystem.h"
#include "Game/View/PlayerView.h"
#include "Scenes/MainMenuScene.h"
#include "cocos2d.h"
#include <algorithm>
#include <cmath>
#include <string>

using namespace cocos2d;

namespace Controllers {

// registerCommonInputHandlers：在指定节点上注册键盘/鼠标/触摸监听，作为统一输入入口。
// - 同步更新内部 UI/背包/作物系统指针，并根据当前面板状态决定哪些输入可用。
void PlayerController::registerCommonInputHandlers(
    Node* ownerNode,
    UIController* ui,
    std::shared_ptr<Game::Inventory> inventory,
    CropSystem* cropSystem,
    bool enableToolOnSpace,
    bool enableToolOnLeftClick,
    std::function<void()> onSpacePressed,
    std::function<void(EventKeyboard::KeyCode)> onKeyPressedHook,
    std::function<void(EventMouse*)> onMouseDownHook) {
    _inputOwner = ownerNode;
    _ui = ui;
    _inventory = std::move(inventory);
    _cropSystem = cropSystem;
    _enableToolOnSpace = enableToolOnSpace;
    _enableToolOnLeftClick = enableToolOnLeftClick;
    _onSpacePressed = std::move(onSpacePressed);
    _onKeyPressedHook = std::move(onKeyPressedHook);
    _onMouseDownHook = std::move(onMouseDownHook);

    if (!_inputOwner) return;
    auto dispatcher = _inputOwner->getEventDispatcher();
    if (!dispatcher) return;

    if (_kbListener) {
        dispatcher->removeEventListener(_kbListener);
        _kbListener = nullptr;
    }
    if (_mouseListener) {
        dispatcher->removeEventListener(_mouseListener);
        _mouseListener = nullptr;
    }
    if (_touchListener) {
        dispatcher->removeEventListener(_touchListener);
        _touchListener = nullptr;
    }

    _kbListener = EventListenerKeyboard::create();
    _kbListener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*) {
        bool chestOpen = _ui && _ui->isChestPanelVisible();
        bool craftOpen = _ui && _ui->isCraftPanelVisible();
        bool storeOpen = _ui && (_ui->isStorePanelVisible()
                                 || _ui->isAnimalStorePanelVisible()
                                 || _ui->isToolUpgradePanelVisible());
        bool skillOpen = _ui && _ui->isSkillTreePanelVisible();
        bool dialogueOpen = _ui && _ui->isDialogueVisible();
        bool socialOpen = _ui && _ui->isNpcSocialVisible();

        if (code == EventKeyboard::KeyCode::KEY_ESCAPE) {
            if (Game::globalState().fishingActive) {
                return;
            }
            if (_ui) {
                if (_ui->isChestPanelVisible()) _ui->toggleChestPanel(false);
                if (_ui->isCraftPanelVisible()) _ui->toggleCraftPanel(false);
                if (_ui->isStorePanelVisible()) _ui->toggleStorePanel(false);
                if (_ui->isAnimalStorePanelVisible()) _ui->toggleAnimalStorePanel(false);
                if (_ui->isSkillTreePanelVisible()) _ui->toggleSkillTreePanel(false);
                if (_ui->isToolUpgradePanelVisible()) _ui->toggleToolUpgradePanel(false);
                if (_ui->isDialogueVisible()) _ui->hideDialogue();
                if (_ui->isNpcSocialVisible()) _ui->hideNpcSocial();
            }
            if (chestOpen || craftOpen || storeOpen || skillOpen || dialogueOpen || socialOpen) {
                return;
            }
            auto& ws = Game::globalState();
            std::string path = Game::currentSavePath();
            if (path.empty()) {
                if (ws.lastSaveSlot < 1) ws.lastSaveSlot = 1;
                if (ws.lastSaveSlot > 50) ws.lastSaveSlot = 50;
                path = Game::savePathForSlot(ws.lastSaveSlot);
            }
            Game::saveToFile(path);
            auto next = MainMenuScene::createScene();
            auto trans = TransitionFade::create(0.5f, next);
            Director::getInstance()->replaceScene(trans);
            return;
        }

        if (socialOpen) {
            return;
        }
        if (skillOpen) {
            if (code != EventKeyboard::KeyCode::KEY_L) {
                return;
            }
        }

        onKeyPressed(code);
        switch (code) {
            case EventKeyboard::KeyCode::KEY_1: if (_ui) _ui->selectHotbarIndex(0); break;
            case EventKeyboard::KeyCode::KEY_2: if (_ui) _ui->selectHotbarIndex(1); break;
            case EventKeyboard::KeyCode::KEY_3: if (_ui) _ui->selectHotbarIndex(2); break;
            case EventKeyboard::KeyCode::KEY_4: if (_ui) _ui->selectHotbarIndex(3); break;
            case EventKeyboard::KeyCode::KEY_5: if (_ui) _ui->selectHotbarIndex(4); break;
            case EventKeyboard::KeyCode::KEY_6: if (_ui) _ui->selectHotbarIndex(5); break;
            case EventKeyboard::KeyCode::KEY_7: if (_ui) _ui->selectHotbarIndex(6); break;
            case EventKeyboard::KeyCode::KEY_8: if (_ui) _ui->selectHotbarIndex(7); break;
            case EventKeyboard::KeyCode::KEY_9: if (_ui) _ui->selectHotbarIndex(8); break;
            case EventKeyboard::KeyCode::KEY_0: if (_ui) _ui->selectHotbarIndex(9); break;
            case EventKeyboard::KeyCode::KEY_Z: {
                if (chestOpen || craftOpen || storeOpen) break;
                Game::Cheat::grantBasic(_inventory);
                if (_ui) _ui->refreshHotbar();
            } break;
            case EventKeyboard::KeyCode::KEY_F5: {
                if (chestOpen || craftOpen || storeOpen) break;
                auto& ws = Game::globalState();
                std::string path = Game::currentSavePath();
                if (path.empty()) {
                    if (ws.lastSaveSlot < 1) ws.lastSaveSlot = 1;
                    if (ws.lastSaveSlot > 50) ws.lastSaveSlot = 50;
                    path = Game::savePathForSlot(ws.lastSaveSlot);
                }
                Game::saveToFile(path);
                if (_ui && _player && _map) {
                    _ui->popTextAt(_map->getPlayerPosition(_player->getPosition()), "Saved", Color3B::WHITE);
                }
            } break;
            case EventKeyboard::KeyCode::KEY_E: {
                if (chestOpen || craftOpen || storeOpen || skillOpen) break;
                openGlobalChest(_ui);
            } break;
            case EventKeyboard::KeyCode::KEY_C: {
                if (chestOpen || storeOpen || skillOpen || dialogueOpen || socialOpen) break;
                if (_ui) {
                    bool show = !_ui->isCraftPanelVisible();
                    _ui->toggleCraftPanel(show);
                }
            } break;
            case EventKeyboard::KeyCode::KEY_L: {
                if (chestOpen || craftOpen || storeOpen) break;
                if (_ui) {
                    bool show = !_ui->isSkillTreePanelVisible();
                    _ui->toggleSkillTreePanel(show);
                }
            } break;
            case EventKeyboard::KeyCode::KEY_T: {
                if (chestOpen || craftOpen || storeOpen || skillOpen) break;
                auto& ws = Game::globalState();
                if (ws.pendingPassOut) break;
                ws.timeHour += 1;
                if (ws.timeHour >= 24) {
                    ws.timeHour = 24;
                    ws.pendingPassOut = true;
                    ws.timeAccum = 0.0f;
                }
                if (_ui) {
                    _ui->refreshHUD();
                }
                if (_ui && _player && _map) {
                    _ui->popTextAt(_map->getPlayerPosition(_player->getPosition()), "time +1h", Color3B::WHITE);
                }
            } break;
            case EventKeyboard::KeyCode::KEY_X: {
                if (chestOpen || craftOpen || storeOpen || skillOpen) break;
                int tc = 0, tr = 0;
                if (_player && _map) {
                    Vec2 playerPos = _player->getPosition();
                    Vec2 dir = lastDir();
                    auto tgt = _map->targetTile(playerPos, dir);
                    tc = tgt.first;
                    tr = tgt.second;
                    if (_cropSystem) { _cropSystem->advanceCropOnceAt(tc, tr); }
                }
                if (_ui && _player && _map) {
                    _map->refreshCropsVisuals();
                    _ui->popTextAt(_map->getPlayerPosition(_player->getPosition()), "Grow +1", Color3B::YELLOW);
                }
            } break;
            case EventKeyboard::KeyCode::KEY_F: {
                if (chestOpen || craftOpen || storeOpen || skillOpen) break;
                auto& ws = Game::globalState();
                if (_inventory && _inventory->selectedKind() == Game::SlotKind::Item) {
                    auto slot = _inventory->selectedSlot();
                    if (slot.itemQty > 0 && Game::itemEdible(slot.itemType)) {
                        bool ok = _inventory->consumeSelectedItem(1);
                        if (ok) {
                            int e = ws.energy + Game::itemEnergyRestore(slot.itemType);
                            int h = ws.hp + Game::itemHpRestore(slot.itemType);
                            ws.energy = std::min(ws.maxEnergy, e);
                            ws.hp = std::min(ws.maxHp, h);
                            if (_ui) {
                                _ui->refreshHotbar();
                                _ui->refreshHUD();
                            }
                            if (_player && _map && _ui) {
                                _ui->popTextAt(_map->getPlayerPosition(_player->getPosition()), "Ate", Color3B::GREEN);
                            }
                        }
                    }
                }
            } break;
            case EventKeyboard::KeyCode::KEY_Q: {
                if (chestOpen || craftOpen || storeOpen || skillOpen) break;
                if (!_inventory || !_player || !_map) break;
                Vec2 playerPos = _player->getPosition();
                Vec2 dir = lastDir();
                int baseC = 0;
                int baseR = 0;
                _map->worldToTileIndex(playerPos, baseC, baseR);
                int stepC = 0;
                int stepR = 0;
                if (std::abs(dir.x) >= std::abs(dir.y)) {
                    if (dir.x > 0.0f) stepC = 1;
                    if (dir.x < 0.0f) stepC = -1;
                } else {
                    if (dir.y > 0.0f) stepR = 1;
                    if (dir.y < 0.0f) stepR = -1;
                }
                if (stepC == 0 && stepR == 0) {
                    stepR = 1;
                }
                int c1 = baseC + stepC;
                int r1 = baseR + stepR;
                int c2 = baseC + stepC * 2;
                int r2 = baseR + stepR * 2;
                int dropC = c2;
                int dropR = r2;
                if (!_map->inBounds(dropC, dropR)) {
                    if (_map->inBounds(c1, r1)) {
                        dropC = c1;
                        dropR = r1;
                    } else if (_map->inBounds(baseC, baseR)) {
                        dropC = baseC;
                        dropR = baseR;
                    } else {
                        break;
                    }
                }
                auto kind = _inventory->selectedKind();
                if (kind == Game::SlotKind::Item) {
                    int selected = _inventory->selectedIndex();
                    if (selected < 0) break;
                    auto slot = _inventory->selectedSlot();
                    if (slot.itemQty <= 0) break;
                    Game::ItemType type = slot.itemType;
                    bool removed = _inventory->removeOneItemFromSlot(static_cast<std::size_t>(selected));
                    if (!removed) break;
                    _map->spawnDropAt(dropC, dropR, static_cast<int>(type), 1);
                    _map->refreshDropsVisuals();
                    if (_ui) {
                        _ui->refreshHotbar();
                    }
                } else if (kind == Game::SlotKind::Tool) {
                    int selected = _inventory->selectedIndex();
                    if (selected < 0) break;
                    const Game::ToolBase* tConst = _inventory->toolAt(static_cast<std::size_t>(selected));
                    if (!tConst) break;
                    Game::ToolKind tk = tConst->kind();
                    int level = tConst->level();
                    bool cleared = _inventory->clearSlot(static_cast<std::size_t>(selected));
                    if (!cleared) break;
                    int raw = Game::toolDropRaw(tk, level);
                    _map->spawnDropAt(dropC, dropR, raw, 1);
                    _map->refreshDropsVisuals();
                    if (_ui) {
                        _ui->refreshHotbar();
                    }
                }
            } break;
            case EventKeyboard::KeyCode::KEY_SPACE: {
                bool toolUpgradeOpen = _ui && _ui->isToolUpgradePanelVisible();
                if (chestOpen || craftOpen || ((!toolUpgradeOpen) && (storeOpen || skillOpen))) break;
                if (Game::globalState().fishingActive) break;
                if (_onSpacePressed) {
                    _onSpacePressed();
                }
                bool blockTool = false;
                if (_ui) {
                    blockTool = _ui->isDialogueVisible()
                                || _ui->isNpcSocialVisible()
                                || _ui->isChestPanelVisible()
                                || _ui->isCraftPanelVisible()
                                || _ui->isStorePanelVisible()
                                || _ui->isAnimalStorePanelVisible()
                                || _ui->isToolUpgradePanelVisible()
                                || _ui->isSkillTreePanelVisible();
                }
                if (blockTool) break;
                if (_enableToolOnSpace) {
                    bool nearDoor = false;
                    if (_player && _map) {
                        Vec2 p = _player->getPosition();
                        nearDoor = _map->isNearDoor(p)
                                   || _map->isNearMineDoor(p)
                                   || _map->isNearBeachDoor(p)
                                   || _map->isNearFarmDoor(p)
                                   || _map->isNearTownDoor(p);
                    }
                    if (!nearDoor) {
                        auto t = _inventory ? _inventory->selectedTool() : nullptr;
                        if (t && t->kind() != Game::ToolKind::FishingRod) {
                            std::string msg = t->use(
                                _map,
                                _cropSystem,
                                [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); },
                                [this]() -> Vec2 { return lastDir(); },
                                _ui);
                            if (_player && !msg.empty()) {
                                _player->playToolAnimation(t->kind(), t->level());
                            }
                        }
                    }
                }
            } break;
            default: break;
        }
        if (_onKeyPressedHook) {
            _onKeyPressedHook(code);
        }
    };
    _kbListener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) {
        onKeyReleased(code);
    };
    dispatcher->addEventListenerWithSceneGraphPriority(_kbListener, _inputOwner);

    _mouseListener = EventListenerMouse::create();
    _mouseListener->onMouseDown = [this](EventMouse* e) {
        bool chestOpen = _ui && _ui->isChestPanelVisible();
        bool craftOpen = _ui && _ui->isCraftPanelVisible();
        bool storeOpen = _ui && (_ui->isStorePanelVisible() || _ui->isAnimalStorePanelVisible());
        bool skillOpen = _ui && _ui->isSkillTreePanelVisible();
        bool dialogueOpen = _ui && _ui->isDialogueVisible();
        if (_ui && _ui->isNpcSocialVisible()) {
            return;
        }
        if (dialogueOpen) {
            return;
        }
        if (Game::globalState().fishingActive) {
            return;
        }
        if (_ui && _ui->handleHotbarMouseDown(e)) return;
        if (chestOpen || craftOpen || storeOpen || skillOpen) return;
        if (_map && _player) {
            Vec2 clickScene = e->getLocation();
			clickScene.y = Director::getInstance()->getVisibleSize().height - clickScene.y;
            auto parent = dynamic_cast<Node*>(_player)->getParent();
            Vec2 mapPos = parent ? parent->convertToNodeSpace(clickScene) : clickScene;
            _map->setLastClickWorldPos(mapPos);
        }
        if (_enableToolOnLeftClick && e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            auto t = _inventory ? _inventory->selectedTool() : nullptr;
            if (t) {
                std::string msg = t->use(
                    _map,
                    _cropSystem,
                    [this]() -> Vec2 { return _player ? _player->getPosition() : Vec2(); },
                    [this]() -> Vec2 { return lastDir(); },
                    _ui);
                if (_player && !msg.empty()) {
                    _player->playToolAnimation(t->kind(), t->level());
                }
            }
        }
        bool consumed = false;
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            if (_ui && _map && _player) {
                Vec2 p = _player->getPosition();
                Vec2 dir = lastDir();
                if (openChestNearPlayer(_map, _ui, p, dir)) {
                    consumed = true;
                } else if (_ui->handleChestRightClick(e, _map->chests())) {
                    consumed = true;
                }
            }
        }
        if (!consumed && _onMouseDownHook) {
            _onMouseDownHook(e);
        }
        if (_map) {
            _map->clearLastClickWorldPos();
        }
    };
    _mouseListener->onMouseScroll = [this](EventMouse* e) {
        if (Game::globalState().fishingActive) {
            return;
        }
        if (_ui) {
            _ui->handleHotbarScroll(e->getScrollY());
        }
    };
    dispatcher->addEventListenerWithSceneGraphPriority(_mouseListener, _inputOwner);

    _touchListener = EventListenerTouchOneByOne::create();
    _touchListener->onTouchBegan = [this](Touch* t, Event*) {
        if (Game::globalState().fishingActive) {
            return false;
        }
        if (!_ui) return false;
        return _ui->handleHotbarAtPoint(t->getLocation());
    };
    dispatcher->addEventListenerWithSceneGraphPriority(_touchListener, _inputOwner);
}

void PlayerController::onKeyPressed(EventKeyboard::KeyCode code) {
    switch (code) {
        case EventKeyboard::KeyCode::KEY_W:
        case EventKeyboard::KeyCode::KEY_UP_ARROW:    _up = true; break;
        case EventKeyboard::KeyCode::KEY_S:
        case EventKeyboard::KeyCode::KEY_DOWN_ARROW:  _down = true; break;
        case EventKeyboard::KeyCode::KEY_A:
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:  _left = true; break;
        case EventKeyboard::KeyCode::KEY_D:
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: _right = true; break;
        case EventKeyboard::KeyCode::KEY_CTRL:
        case EventKeyboard::KeyCode::KEY_RIGHT_CTRL: {
            ++_toolRangeModifierHeld;
            auto& ws = Game::globalState();
            ws.toolRangeModifier = (_toolRangeModifierHeld > 0);
        } break;
        default: break;
    }
}

void PlayerController::onKeyReleased(EventKeyboard::KeyCode code) {
    switch (code) {
        case EventKeyboard::KeyCode::KEY_W:
        case EventKeyboard::KeyCode::KEY_UP_ARROW:    _up = false; break;
        case EventKeyboard::KeyCode::KEY_S:
        case EventKeyboard::KeyCode::KEY_DOWN_ARROW:  _down = false; break;
        case EventKeyboard::KeyCode::KEY_A:
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:  _left = false; break;
        case EventKeyboard::KeyCode::KEY_D:
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: _right = false; break;
        case EventKeyboard::KeyCode::KEY_CTRL:
        case EventKeyboard::KeyCode::KEY_RIGHT_CTRL: {
            if (_toolRangeModifierHeld > 0) {
                --_toolRangeModifierHeld;
            }
            auto& ws = Game::globalState();
            ws.toolRangeModifier = (_toolRangeModifierHeld > 0);
        } break;
        default: break;
    }
}

void PlayerController::update(float dt) {
    if (!_player || !_map) return;

    if (_movementLocked) {
        _player->setMoving(false);
        _moveHeldDuration = 0.0f;
        _isSprinting = false;
        // 移动被锁定时：保持光标与相机跟随玩家，但不更新玩家位置。
        _map->updateCursor(_player->getPosition(), _lastDir);
        if (_worldNode) {
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto origin = Director::getInstance()->getVisibleOrigin();
            float scale = _worldNode->getScale();
            auto org = _map->getOrigin();
            auto playerPos = _player->getPosition();
            Vec2 screenCenter(origin.x + visibleSize.width * 0.5f, origin.y + visibleSize.height * 0.5f);
            Vec2 cam = screenCenter - (org + playerPos) * scale;
            cocos2d::Size mapSize = _map->getContentSize();
            float mapW = mapSize.width;
            float mapH = mapSize.height;
            float minX = (origin.x + visibleSize.width) - (org.x + mapW) * scale;
            float maxX = origin.x - org.x * scale;
            float minY = (origin.y + visibleSize.height) - (org.y + mapH) * scale;
            float maxY = origin.y - org.y * scale;
            if (mapW * scale <= visibleSize.width) {
                cam.x = (origin.x + visibleSize.width * 0.5f) - (org.x + mapW * 0.5f) * scale;
            } else {
                cam.x = std::max(minX, std::min(maxX, cam.x));
            }
            if (mapH * scale <= visibleSize.height) {
                cam.y = (origin.y + visibleSize.height * 0.5f) - (org.y + mapH * 0.5f) * scale;
            } else {
                cam.y = std::max(minY, std::min(maxY, cam.y));
            }
            _worldNode->setPosition(cam);
        }
        if (_map) { _map->sortActorWithEnvironment(_player); }
        return;
    }

    // 冲刺判定：任意方向键持续按住超过阈值则进入冲刺状态。
    bool movementHeld = (_up || _down || _left || _right);
    if (movementHeld) {
        _moveHeldDuration += dt;
        _isSprinting = (_moveHeldDuration >= _sprintThreshold);
    } else {
        _moveHeldDuration = 0.0f;
        _isSprinting = false;
    }

    float dx = 0.0f, dy = 0.0f;
    if (_left)  dx -= 1.0f;
    if (_right) dx += 1.0f;
    if (_down)  dy -= 1.0f;
    if (_up)    dy += 1.0f;

    Vec2 dir(dx, dy);
    if (dir.lengthSquared() > 0.0f) {
        dir.normalize();
        _lastDir = dir;
    }

    bool isMoving = (dx != 0 || dy != 0);
    _player->setMoving(isMoving);
    if (isMoving) {
        if (std::abs(dx) > std::abs(dy)) {
            _player->setDirection(dx > 0 ? Game::IPlayerView::Direction::RIGHT
                                          : Game::IPlayerView::Direction::LEFT);
        } else {
            _player->setDirection(dy > 0 ? Game::IPlayerView::Direction::UP
                                          : Game::IPlayerView::Direction::DOWN);
        }
    }
    _player->updateAnimation(dt);

    float speed = (_isSprinting ? _sprintSpeed : _baseSpeed) * _moveSpeedMultiplier;
    Vec2 delta = dir * speed * dt;
    Vec2 next = _player->getPosition() + delta;

    Vec2 clamped = _map->clampPosition(_player->getPosition(), next, /*radius*/8.0f);
    _player->setPosition(clamped);
    if (auto* pv = dynamic_cast<Game::PlayerView*>(_player)) {
        pv->syncUpperLayerPosition();
    }

    // 更新农场光标与相机跟随，使玩家始终保持在视野中心附近。
    _map->updateCursor(_player->getPosition(), _lastDir);

    if (_worldNode) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        float scale = _worldNode->getScale();
        auto org = _map->getOrigin();
        auto playerPos = _player->getPosition();
        Vec2 screenCenter(origin.x + visibleSize.width * 0.5f, origin.y + visibleSize.height * 0.5f);
        Vec2 cam = screenCenter - (org + playerPos) * scale;

        cocos2d::Size mapSize = _map->getContentSize();
        float mapW = mapSize.width;
        float mapH = mapSize.height;

        float minX = (origin.x + visibleSize.width) - (org.x + mapW) * scale;
        float maxX = origin.x - org.x * scale;
        float minY = (origin.y + visibleSize.height) - (org.y + mapH) * scale;
        float maxY = origin.y - org.y * scale;

        if (mapW * scale <= visibleSize.width) {
            cam.x = (origin.x + visibleSize.width * 0.5f) - (org.x + mapW * 0.5f) * scale;
        } else {
            cam.x = std::max(minX, std::min(maxX, cam.x));
        }

        if (mapH * scale <= visibleSize.height) {
            cam.y = (origin.y + visibleSize.height * 0.5f) - (org.y + mapH * 0.5f) * scale;
        } else {
            cam.y = std::max(minY, std::min(maxY, cam.y));
        }
        _worldNode->setPosition(cam);
    }

    if (_map) {
        _map->sortActorWithEnvironment(_player);
        if (auto* pv = dynamic_cast<Game::PlayerView*>(_player)) {
            pv->syncUpperLayerZ();
        }
    }
}
// namespace Controllers
} // namespace Controllers
