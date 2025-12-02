/**
 * RoomScene: 室内场景，靠近门口显示提示，按空格切换到农场。
 */
#include "Scenes/RoomScene.h"
#include "Scenes/GameScene.h"
#include "cocos2d.h"
#include "Game/Inventory.h"
#include "Game/Tool.h"
#include "Game/Item.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"
// UI 控件（Button）
#include "ui/CocosGUI.h"

USING_NS_CC;

Scene* RoomScene::createScene() {
    return RoomScene::create();
}

bool RoomScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 标题
    auto title = Label::createWithTTF("Room", "fonts/Marker Felt.ttf", 30);
    if (title) {
        title->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height * 0.85f));
        this->addChild(title, 1);
    }

    // 房间绘制节点
    _roomDraw = DrawNode::create();
    this->addChild(_roomDraw, 0);
    buildRoom();

    // 玩家（与 GameScene 一致的占位图形：方块）
    _player = DrawNode::create();
    const float size = 16.f;
    Vec2 verts[4] = { Vec2(-size, -size), Vec2(size, -size), Vec2(size, size), Vec2(-size, size) };
    _player->drawSolidPoly(verts, 4, Color4F(0.2f, 0.7f, 0.9f, 1.0f));
    // 初始位置：房间中上部
    _player->setPosition(Vec2(_roomRect.getMidX(), _roomRect.getMidY() + 80));
    this->addChild(_player, 2);

    // 物品栏与热键栏（共享全局背包）
    auto &ws = Game::globalState();
    if (!ws.inventory) {
        ws.inventory = std::make_shared<Game::Inventory>(GameConfig::TOOLBAR_SLOTS);
        ws.inventory->setTool(0, Game::makeTool(Game::ToolType::Axe));
        ws.inventory->setTool(1, Game::makeTool(Game::ToolType::Hoe));
        ws.inventory->setTool(2, Game::makeTool(Game::ToolType::Pickaxe));
        ws.inventory->setTool(3, Game::makeTool(Game::ToolType::WateringCan));
    }
    _inventory = ws.inventory;
    if (_inventory) { _inventory->selectIndex(ws.selectedIndex); }
    buildHotbarUI();

    // HUD: time and energy (English)
    buildHUD();

    // 门口交互提示（初始隐藏）
    _doorPrompt = Label::createWithTTF("Press Space to Exit", "fonts/Marker Felt.ttf", 20);
    if (_doorPrompt) {
        _doorPrompt->setColor(Color3B::YELLOW);
        _doorPrompt->setVisible(false);
        this->addChild(_doorPrompt, 3);
    }

    // 摆设标签
    _bedLabel = Label::createWithTTF("Bed", "fonts/Marker Felt.ttf", 18);
    _tableLabel = Label::createWithTTF("Table", "fonts/Marker Felt.ttf", 18);
    if (_bedLabel) {
        _bedLabel->setColor(Color3B::WHITE);
        _bedLabel->setPosition(Vec2(_bedRect.getMidX(), _bedRect.getMaxY() + 18));
        this->addChild(_bedLabel, 2);
    }
    if (_tableLabel) {
        _tableLabel->setColor(Color3B::WHITE);
        _tableLabel->setPosition(Vec2(_tableRect.getMidX(), _tableRect.getMaxY() + 18));
        this->addChild(_tableLabel, 2);
    }

    // 床交互提示（初始隐藏）
    _bedPrompt = Label::createWithTTF("Press Space to Sleep", "fonts/Marker Felt.ttf", 20);
    if (_bedPrompt) {
        _bedPrompt->setColor(Color3B::YELLOW);
        _bedPrompt->setVisible(false);
        this->addChild(_bedPrompt, 3);
    }

    // 室内箱子绘制与提示
    _chestDraw = DrawNode::create();
    this->addChild(_chestDraw, 1);
    _houseChests = ws.houseChests;
    refreshChestsVisuals();
    _chestPrompt = Label::createWithTTF("Press Space to Deposit", "fonts/Marker Felt.ttf", 20);
    if (_chestPrompt) {
        _chestPrompt->setColor(Color3B::YELLOW);
        _chestPrompt->setVisible(false);
    this->addChild(_chestPrompt, 3);
    }

    // 室内箱子面板
    buildChestUI();

    // 键盘输入（与 GameScene 保持一致）
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*) {
        switch (code) {
            case EventKeyboard::KeyCode::KEY_W:
            case EventKeyboard::KeyCode::KEY_UP_ARROW:   _up = true; break;
            case EventKeyboard::KeyCode::KEY_S:
            case EventKeyboard::KeyCode::KEY_DOWN_ARROW: _down = true; break;
            case EventKeyboard::KeyCode::KEY_A:
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW: _left = true; break;
            case EventKeyboard::KeyCode::KEY_D:
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:_right = true; break;
            // 热键栏选择（持久化选中索引）
            case EventKeyboard::KeyCode::KEY_1:          if (_inventory) { _inventory->selectIndex(0); Game::globalState().selectedIndex = 0; refreshHotbarUI(); } break;
            case EventKeyboard::KeyCode::KEY_2:          if (_inventory) { _inventory->selectIndex(1); Game::globalState().selectedIndex = 1; refreshHotbarUI(); } break;
            case EventKeyboard::KeyCode::KEY_3:          if (_inventory) { _inventory->selectIndex(2); Game::globalState().selectedIndex = 2; refreshHotbarUI(); } break;
            case EventKeyboard::KeyCode::KEY_4:          if (_inventory) { _inventory->selectIndex(3); Game::globalState().selectedIndex = 3; refreshHotbarUI(); } break;
            case EventKeyboard::KeyCode::KEY_5:          if (_inventory) { _inventory->selectIndex(4); Game::globalState().selectedIndex = 4; refreshHotbarUI(); } break;
            case EventKeyboard::KeyCode::KEY_6:          if (_inventory) { _inventory->selectIndex(5); Game::globalState().selectedIndex = 5; refreshHotbarUI(); } break;
            case EventKeyboard::KeyCode::KEY_7:          if (_inventory) { _inventory->selectIndex(6); Game::globalState().selectedIndex = 6; refreshHotbarUI(); } break;
            case EventKeyboard::KeyCode::KEY_8:          if (_inventory) { _inventory->selectIndex(7); Game::globalState().selectedIndex = 7; refreshHotbarUI(); } break;
            case EventKeyboard::KeyCode::KEY_9:          if (_inventory) { _inventory->selectIndex(8); Game::globalState().selectedIndex = 8; refreshHotbarUI(); } break;
            case EventKeyboard::KeyCode::KEY_0:          if (_inventory) { _inventory->selectIndex(9); Game::globalState().selectedIndex = 9; refreshHotbarUI(); } break;
            case EventKeyboard::KeyCode::KEY_SPACE:
                if (_nearDoor && !_transitioning) {
                    _transitioning = true;
                    auto farm = GameScene::create();
                    // 切到农场并出生在门外侧
                    farm->setSpawnAtFarmEntrance();
                    auto trans = TransitionFade::create(0.6f, farm);
                    Director::getInstance()->replaceScene(trans);
                } else if (_nearBed) {
                    // 睡觉：能量回满并进入下一天（每季30天，四季循环）
                    auto &ws = Game::globalState();
                    ws.energy = ws.maxEnergy;
                    ws.dayOfSeason += 1;
                    if (ws.dayOfSeason > 30) { ws.dayOfSeason = 1; ws.seasonIndex = (ws.seasonIndex + 1) % 4; }
                    ws.timeHour = 6; // wake up at 06:00
                    ws.timeMinute = 0;
                    ws.timeAccum = 0.0f;
                    refreshHUD();
                    auto seasonName = [](int idx){
                        switch (idx % 4) { case 0: return "Spring"; case 1: return "Summer"; case 2: return "Fall"; default: return "Winter"; }
                    };
                    auto pop = Label::createWithTTF(StringUtils::format("New Day: %s Day %d, %02d:%02d", seasonName(ws.seasonIndex), ws.dayOfSeason, ws.timeHour, ws.timeMinute), "fonts/Marker Felt.ttf", 20);
                    pop->setColor(Color3B::WHITE);
                    auto pos = _player ? _player->getPosition() : Vec2(0,0);
                    pop->setPosition(pos + Vec2(0, 26));
                    this->addChild(pop, 3);
                    auto seq = Sequence::create(FadeOut::create(0.8f), RemoveSelf::create(), nullptr);
                    pop->runAction(seq);
                } else if (_inventory && _inventory->selectedKind() == Game::SlotKind::Item) {
                    const auto &slot = _inventory->selectedSlot();
                    if (slot.itemType == Game::ItemType::Chest) {
                        // 放置箱子：避免与门、床、桌子重叠
                        Vec2 p = _player->getPosition();
                        Rect chestRect(p.x - 20, p.y - 20, 40, 40);
                        bool blocked = _doorRect.containsPoint(p) || chestRect.intersectsRect(_bedRect) || chestRect.intersectsRect(_tableRect);
                        if (!blocked) {
                            Game::Chest chest{ p, Game::Bag{} };
                            _houseChests.push_back(chest);
                            Game::globalState().houseChests = _houseChests;
                            refreshChestsVisuals();
                            _inventory->removeItems(Game::ItemType::Chest, 1);
                            refreshHotbarUI();
                            auto pop2 = Label::createWithTTF("Placed Chest", "fonts/Marker Felt.ttf", 20);
                            pop2->setColor(Color3B::YELLOW);
                            pop2->setPosition(p + Vec2(0, 26));
                            this->addChild(pop2, 3);
                            auto seq2 = Sequence::create(FadeOut::create(0.8f), RemoveSelf::create(), nullptr);
                            pop2->runAction(seq2);
                        }
                    } else {
                        // 近箱子则存入最近的箱子
                        checkChestRegion();
                        if (_nearChest && slot.itemQty > 0) {
                            Vec2 p = _player->getPosition();
                            int idx = -1; float best = 1e9f;
                            for (int i=0;i<(int)_houseChests.size();++i) {
                                float d = p.distance(_houseChests[i].pos);
                                if (d < best) { best = d; idx = i; }
                            }
                            if (idx >= 0) {
                                int qty = slot.itemQty;
                                _houseChests[idx].bag.add(slot.itemType, qty);
                                Game::globalState().houseChests = _houseChests;
                                _inventory->consumeSelectedItem(qty);
                                refreshHotbarUI();
                                auto pop3 = Label::createWithTTF("Stored Items", "fonts/Marker Felt.ttf", 20);
                                pop3->setColor(Color3B::YELLOW);
                                pop3->setPosition(p + Vec2(0, 26));
                                this->addChild(pop3, 3);
                                auto seq3 = Sequence::create(FadeOut::create(0.6f), RemoveSelf::create(), nullptr);
                                pop3->runAction(seq3);
                            }
                        }
                    }
                } else {
                    // 室内不支持使用工具，给予提示反馈
                    auto pop = Label::createWithTTF("No effect indoors", "fonts/Marker Felt.ttf", 20);
                    pop->setColor(Color3B::YELLOW);
                    auto pos = _player ? _player->getPosition() : Vec2(0,0);
                    pop->setPosition(pos + Vec2(0, 26));
                    this->addChild(pop, 3);
                    auto seq = Sequence::create(FadeOut::create(0.6f), RemoveSelf::create(), nullptr);
                    pop->runAction(seq);
                }
                break;
            case EventKeyboard::KeyCode::KEY_C: {
                // 室内箱子面板开关
                if (_chestPanel && _chestPanel->isVisible()) {
                    _chestPanel->setVisible(false);
                    break;
                }
                checkChestRegion();
                if (_nearChest) {
                    Vec2 p = _player->getPosition();
                    int idx = -1; float best = 1e9f;
                    for (int i=0;i<(int)_houseChests.size();++i) {
                        float d = p.distance(_houseChests[i].pos);
                        if (d < best) { best = d; idx = i; }
                    }
                    if (idx >= 0) { showChestPanel(idx); }
                } else {
                    auto pop = Label::createWithTTF("No chest nearby", "fonts/Marker Felt.ttf", 20);
                    pop->setColor(Color3B::RED);
                    auto pos = _player ? _player->getPosition() : Vec2(0,0);
                    pop->setPosition(pos + Vec2(0, 26));
                    this->addChild(pop, 3);
                    auto seq = Sequence::create(FadeOut::create(0.6f), RemoveSelf::create(), nullptr);
                    pop->runAction(seq);
                }
                break;
            }
            case EventKeyboard::KeyCode::KEY_Z: {
                // 作弊：各类基础资源 +99
                if (_inventory) {
                    for (auto t : { Game::ItemType::Wood, Game::ItemType::Stone, Game::ItemType::Fiber, Game::ItemType::Chest }) {
                        _inventory->addItems(t, 99);
                    }
                    refreshHotbarUI();
                    auto pop = Label::createWithTTF("Cheat: +99 All", "fonts/Marker Felt.ttf", 20);
                    pop->setColor(Color3B::YELLOW);
                    auto pos = _player ? _player->getPosition() : Vec2(0,0);
                    pop->setPosition(pos + Vec2(0, 26));
                    this->addChild(pop, 3);
                    auto seq = Sequence::create(FadeOut::create(0.6f), RemoveSelf::create(), nullptr);
                    pop->runAction(seq);
                }
                break;
            }
            case EventKeyboard::KeyCode::KEY_ESCAPE:
                if (_chestPanel) _chestPanel->setVisible(false);
                break;
            case EventKeyboard::KeyCode::KEY_F: {
                // 吃东西：选中为可食用物品时，消耗并回复能量
                if (_inventory && _inventory->selectedKind() == Game::SlotKind::Item) {
                    const auto &slot = _inventory->selectedSlot();
                    if (Game::itemEdible(slot.itemType)) {
                        bool ok = _inventory->consumeSelectedItem(1);
                        if (ok) {
                            auto &ws2 = Game::globalState();
                            int recover = GameConfig::ENERGY_RECOVER_FIBER;
                            ws2.energy = std::min(ws2.maxEnergy, ws2.energy + recover);
                            refreshHotbarUI();
                            refreshHUD();
                            auto pop = Label::createWithTTF(std::string("Ate ") + Game::itemName(slot.itemType) + StringUtils::format(" +%d Energy", recover), "fonts/Marker Felt.ttf", 20);
                            pop->setColor(Color3B::YELLOW);
                            auto pos = _player ? _player->getPosition() : Vec2(0,0);
                            pop->setPosition(pos + Vec2(0, 26));
                            this->addChild(pop, 3);
                            auto seq = Sequence::create(FadeOut::create(0.6f), RemoveSelf::create(), nullptr);
                            pop->runAction(seq);
                        }
                    }
                }
                break;
            }
            default: break;
        }
    };
    listener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) {
        switch (code) {
            case EventKeyboard::KeyCode::KEY_W:
            case EventKeyboard::KeyCode::KEY_UP_ARROW:   _up = false; break;
            case EventKeyboard::KeyCode::KEY_S:
            case EventKeyboard::KeyCode::KEY_DOWN_ARROW: _down = false; break;
            case EventKeyboard::KeyCode::KEY_A:
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW: _left = false; break;
            case EventKeyboard::KeyCode::KEY_D:
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:_right = false; break;
            default: break;
        }
        if (!(_up || _down || _left || _right)) {
            _moveHeldDuration = 0.0f;
            _isSprinting = false;
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 鼠标：滚轮切换，点击热键栏选择
    auto mouse = EventListenerMouse::create();
    mouse->onMouseScroll = [this](EventMouse* e){
        if (!_inventory) return;
        float dy = e->getScrollY();
        // 方向调整：滚轮上滚（dy>0）选择下一个；下滚（dy<0）选择上一个
        if (dy > 0) {
            _inventory->next();
        } else if (dy < 0) {
            _inventory->prev();
        } else {
            return;
        }
        Game::globalState().selectedIndex = _inventory->selectedIndex();
        refreshHotbarUI();
    };
    mouse->onMouseDown = [this](EventMouse* e){
        // 右键：靠近箱子时打开交互面板
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            checkChestRegion();
            if (_nearChest) {
                Vec2 p = _player ? _player->getPosition() : Vec2::ZERO;
                int idx = -1; float best = 1e9f;
                for (int i=0;i<(int)_houseChests.size();++i) {
                    float d = p.distance(_houseChests[i].pos);
                    if (d < best) { best = d; idx = i; }
                }
                if (idx >= 0) { showChestPanel(idx); }
            } else {
                auto pop = Label::createWithTTF("No chest nearby", "fonts/Marker Felt.ttf", 20);
                pop->setColor(Color3B::RED);
                auto pos = _player ? _player->getPosition() : Vec2::ZERO;
                pop->setPosition(pos + Vec2(0, 26));
                this->addChild(pop, 3);
                auto seq = Sequence::create(FadeOut::create(0.6f), RemoveSelf::create(), nullptr);
                pop->runAction(seq);
            }
            return;
        }
        if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
        auto p = e->getLocation();
        // 不再在面板外点击时关闭面板，以便拖拽操作

        // 选择热键栏槽位，或开始拖拽到箱子面板
        if (_inventory && _hotbarNode) {
            auto local = _hotbarNode->convertToNodeSpace(p);
            int slots = static_cast<int>(_inventory->size());
            if (slots > 0) {
                float slotW = 80.0f, slotH = 32.0f, padding = 6.0f, hitMarginY = 8.0f;
                float totalWidth = slots * slotW + (slots - 1) * padding;
                if (!(local.y < -(slotH/2 + hitMarginY) || local.y > (slotH/2 + hitMarginY))) {
                    for (int i = 0; i < slots; ++i) {
                        float cx = -totalWidth/2 + i * (slotW + padding) + slotW/2;
                        float minx = cx - slotW/2;
                        float maxx = cx + slotW/2;
                        if (local.x >= minx && local.x <= maxx) {
                            _inventory->selectIndex(i);
                            Game::globalState().selectedIndex = i;
                            refreshHotbarUI();
                            if (_chestPanel && _chestPanel->isVisible() && _inventory->isItem(i)) {
                                auto st = _inventory->itemAt(i);
                                _dragging = true;
                                _dragSource = DragSource::Inventory;
                                _dragSlotIndex = i;
                                _dragType = st.type;
                                _dragQty = st.quantity;
                                if (!_dragGhost) {
                                    _dragGhost = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
                                    this->addChild(_dragGhost, 5);
                                }
                                _dragGhost->setString(StringUtils::format("%s x%d", Game::itemName(st.type), st.quantity));
                                _dragGhost->setColor(Color3B::YELLOW);
                                _dragGhost->setPosition(p);
                            }
                            return;
                        }
                    }
                }
            }
        }

        // 从箱子面板的行开始拖拽到热键栏（整堆），以行的实际 Y 值构造“整行”命中矩形
        if (_chestPanel && _chestPanel->isVisible() && _activeChestIdx >= 0 && _activeChestIdx < (int)_houseChests.size()) {
            auto &bag = _houseChests[_activeChestIdx].bag;
            Vec2 listLocal = _chestListNode ? _chestListNode->convertToNodeSpace(p) : _chestPanel->convertToNodeSpace(p);
            float xLeft = -_chestPanelW/2 + 10.0f;
            float xRight = _chestPanelW/2 - 10.0f;
            for (int idx = 0; idx < (int)_withdrawRows.size(); ++idx) {
                const auto &row = _withdrawRows[idx];
                if (!row.countLabel || !row.nameLabel) continue;
                float y = row.nameLabel->getPositionY();
                float rowH = std::max(row.nameLabel->getContentSize().height, row.countLabel->getContentSize().height) + 16.0f;
                Rect rowRect(xLeft, y - rowH * 0.5f, xRight - xLeft, rowH);
                if (rowRect.containsPoint(listLocal)) {
                    auto type = row.type;
                    int have = bag.count(type);
                    if (have > 0) {
                        _dragging = true;
                        _dragSource = DragSource::Chest;
                        _dragType = type;
                        _dragQty = have;
                        if (!_dragGhost) {
                            _dragGhost = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
                            this->addChild(_dragGhost, 5);
                        }
                        _dragGhost->setString(StringUtils::format("%s x%d", Game::itemName(type), have));
                        _dragGhost->setColor(Color3B::YELLOW);
                        _dragGhost->setPosition(p);
                    }
                    return;
                }
            }
        }
    };
    mouse->onMouseMove = [this](EventMouse* e){
        if (_dragging && _dragGhost) {
            _dragGhost->setPosition(e->getLocation());
        }
    };
    mouse->onMouseUp = [this](EventMouse* e){
        if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
        if (!_dragging) return;
        auto p = e->getLocation();
        if (_dragSource == DragSource::Inventory) {
            // 拖到箱子面板进行存入（直接使用拖拽记录的类型与数量）
            if (_chestPanel && _chestPanel->isVisible() && _activeChestIdx >= 0 && _activeChestIdx < (int)_houseChests.size()) {
                auto chestLocal = _chestPanel->convertToNodeSpace(p);
                bool inChest = (chestLocal.x >= -_chestPanelW/2 && chestLocal.x <= _chestPanelW/2 &&
                                chestLocal.y >= -_chestPanelH/2 && chestLocal.y <= _chestPanelH/2);
                if (inChest && _inventory) {
                    _houseChests[_activeChestIdx].bag.add(_dragType, _dragQty);
                    Game::globalState().houseChests = _houseChests;
                    _inventory->selectIndex(_dragSlotIndex);
                    _inventory->consumeSelectedItem(_dragQty);
                    refreshChestUI();
                    refreshHotbarUI();
                }
            }
        } else if (_dragSource == DragSource::Chest) {
            // 拖到热键栏进行取出
            if (_hotbarNode && _inventory) {
                auto hbLocal = _hotbarNode->convertToNodeSpace(p);
                int slots = static_cast<int>(_inventory->size());
                float slotW = 80.0f, slotH = 32.0f, padding = 6.0f, hitY = 8.0f;
                float totalW = slots * slotW + (slots - 1) * padding;
                bool inHotbar = !(hbLocal.y < -(slotH/2 + hitY) || hbLocal.y > (slotH/2 + hitY)) &&
                                (hbLocal.x >= -totalW/2 - 10 && hbLocal.x <= totalW/2 + 10);
                if (inHotbar) {
                    attemptWithdraw(_dragType, _dragQty);
                }
            }
        }
        _dragging = false;
        _dragSource = DragSource::None;
        _dragSlotIndex = -1;
        _dragQty = 0;
        if (_dragGhost) { _dragGhost->removeFromParent(); _dragGhost = nullptr; }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);

    // 兼容触摸输入：部分平台只派发触摸事件
    auto touch = EventListenerTouchOneByOne::create();
    touch->setSwallowTouches(false);
    touch->onTouchBegan = [this](Touch* t, Event*){
        if (!_inventory || !_hotbarNode) return false;
        auto p = t->getLocation();
        auto local = _hotbarNode->convertToNodeSpace(p);
        int slots = static_cast<int>(_inventory->size());
        if (slots <= 0) return false;
        float slotW = 80.0f;
        float slotH = 32.0f;
        float padding = 6.0f;
        float hitMarginY = 8.0f;
        float totalWidth = slots * slotW + (slots - 1) * padding;
        if (local.y < -(slotH/2 + hitMarginY) || local.y > (slotH/2 + hitMarginY)) return false;
        for (int i = 0; i < slots; ++i) {
            float cx = -totalWidth/2 + i * (slotW + padding) + slotW/2;
            float minx = cx - slotW/2;
            float maxx = cx + slotW/2;
            if (local.x >= minx && local.x <= maxx) {
                _inventory->selectIndex(i);
                Game::globalState().selectedIndex = i;
                refreshHotbarUI();
                return true;
            }
        }
        return false;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touch, this);

    this->scheduleUpdate();
    return true;
}

void RoomScene::buildRoom() {
    if (!_roomDraw) return;
    _roomDraw->clear();

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 房间大小：屏幕中间，略小于屏幕
    float roomW = std::min(visibleSize.width * 0.70f, 900.0f);
    float roomH = std::min(visibleSize.height * 0.70f, 520.0f);
    float rx = origin.x + (visibleSize.width - roomW) * 0.5f;
    float ry = origin.y + (visibleSize.height - roomH) * 0.5f;
    _roomRect = Rect(rx, ry, roomW, roomH);

    // 门口区域：房间下边中间的一段缺口
    float doorW = 100.0f;
    float doorH = 24.0f;
    float dx = _roomRect.getMidX() - doorW * 0.5f;
    float dy = _roomRect.getMinY() + 2.0f; // 靠近房间底部
    _doorRect = Rect(dx, dy, doorW, doorH);

    // 地板
    _roomDraw->drawSolidRect(_roomRect.origin,
                             Vec2(_roomRect.getMaxX(), _roomRect.getMaxY()),
                             Color4F(0.85f, 0.75f, 0.60f, 1.0f));

    // 墙体边框（底边在门口处留缺口）
    Vec2 tl(_roomRect.getMinX(), _roomRect.getMaxY());
    Vec2 tr(_roomRect.getMaxX(), _roomRect.getMaxY());
    Vec2 bl(_roomRect.getMinX(), _roomRect.getMinY());
    Vec2 br(_roomRect.getMaxX(), _roomRect.getMinY());

    Color4F wall(0.f, 0.f, 0.f, 0.55f);
    _roomDraw->drawLine(tl, tr, wall); // 顶边
    _roomDraw->drawLine(tl, bl, wall); // 左边
    _roomDraw->drawLine(tr, br, wall); // 右边
    // 底边分两段：左到门左；门右到右
    Vec2 dl(_doorRect.getMinX(), _roomRect.getMinY());
    Vec2 dr(_doorRect.getMaxX(), _roomRect.getMinY());
    _roomDraw->drawLine(bl, dl, wall);
    _roomDraw->drawLine(dr, br, wall);

    // 门区域视觉提示
    _roomDraw->drawSolidRect(_doorRect.origin,
                             Vec2(_doorRect.getMaxX(), _doorRect.getMaxY()),
                             Color4F(0.75f, 0.75f, 0.20f, 0.75f));

    // 摆设：床与桌子
    // 床：房间左上角
    float bedW = 120.0f, bedH = 60.0f;
    _bedRect = Rect(_roomRect.getMinX() + 24.0f,
                    _roomRect.getMaxY() - bedH - 24.0f,
                    bedW, bedH);
    _roomDraw->drawSolidRect(_bedRect.origin,
                             Vec2(_bedRect.getMaxX(), _bedRect.getMaxY()),
                             Color4F(0.85f, 0.85f, 0.95f, 1.0f));
    _roomDraw->drawRect(_bedRect.origin,
                        Vec2(_bedRect.getMaxX(), _bedRect.getMaxY()),
                        Color4F(0.2f, 0.2f, 0.3f, 1.0f));

    // 桌子：房间中下部偏左
    float tableW = 140.0f, tableH = 70.0f;
    _tableRect = Rect(_roomRect.getMinX() + _roomRect.size.width * 0.25f,
                      _roomRect.getMinY() + _roomRect.size.height * 0.35f,
                      tableW, tableH);
    _roomDraw->drawSolidRect(_tableRect.origin,
                             Vec2(_tableRect.getMaxX(), _tableRect.getMaxY()),
                             Color4F(0.70f, 0.50f, 0.35f, 1.0f));
    _roomDraw->drawRect(_tableRect.origin,
                        Vec2(_tableRect.getMaxX(), _tableRect.getMaxY()),
                        Color4F(0.25f, 0.15f, 0.10f, 1.0f));
}

void RoomScene::update(float dt) {
    // time progression: 1 real second -> 1 game minute
    auto &wsTime = Game::globalState();
    bool timeChanged = false;
    wsTime.timeAccum += dt;
    while (wsTime.timeAccum >= GameConfig::REAL_SECONDS_PER_GAME_MINUTE) {
        wsTime.timeAccum -= GameConfig::REAL_SECONDS_PER_GAME_MINUTE;
        wsTime.timeMinute += 1;
        if (wsTime.timeMinute >= 60) {
            wsTime.timeMinute = 0;
            wsTime.timeHour += 1;
            if (wsTime.timeHour >= 24) {
                wsTime.timeHour = 0;
                wsTime.dayOfSeason += 1;
                if (wsTime.dayOfSeason > 30) { wsTime.dayOfSeason = 1; wsTime.seasonIndex = (wsTime.seasonIndex + 1) % 4; }
            }
        }
        timeChanged = true;
    }
    if (timeChanged) { refreshHUD(); }

    // 加速计时：按住任意方向键
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

    if (dx == 0.0f && dy == 0.0f) {
        checkDoorRegion();
        checkBedRegion();
        checkChestRegion();
        return;
    }

    Vec2 dir(dx, dy);
    dir.normalize();
    float speed = _isSprinting ? _sprintSpeed : _baseSpeed;
    Vec2 delta = dir * speed * dt;
    Vec2 next = _player->getPosition() + delta;

    // 限制在房间矩形内运动（边界夹紧）
    float pad = 16.0f;
    next.x = std::max(_roomRect.getMinX() + pad, std::min(_roomRect.getMaxX() - pad, next.x));
    next.y = std::max(_roomRect.getMinY() + pad, std::min(_roomRect.getMaxY() - pad, next.y));
    _player->setPosition(next);

    checkDoorRegion();
    checkBedRegion();
    checkChestRegion();
}

void RoomScene::checkDoorRegion() {
    if (!_player) return;
    Vec2 p = _player->getPosition();
    _nearDoor = _doorRect.containsPoint(p);
    if (_doorPrompt) {
        _doorPrompt->setVisible(_nearDoor);
        if (_nearDoor) {
            _doorPrompt->setString("Press Space to Exit");
            _doorPrompt->setPosition(p + Vec2(0, 26));
        }
    }
}

void RoomScene::setSpawnInsideDoor() {
    if (!_player) return;
    // 出生在门内侧，稍微离开门一点，避免提示一直显示
    float offsetY = 28.0f;
    Vec2 spawn(_doorRect.getMidX(), _doorRect.getMinY() + _doorRect.size.height + offsetY);
    _player->setPosition(spawn);
}
void RoomScene::buildHUD() {
    auto &ws = Game::globalState();
    if (ws.maxEnergy <= 0) ws.maxEnergy = GameConfig::ENERGY_MAX;
    if (ws.energy < 0 || ws.energy > ws.maxEnergy) ws.energy = ws.maxEnergy;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    if (!_hudTimeLabel) {
        _hudTimeLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
        _hudTimeLabel->setColor(Color3B::WHITE);
        _hudTimeLabel->setAnchorPoint(Vec2(1,1));
        float pad = 10.0f;
        _hudTimeLabel->setPosition(Vec2(origin.x + visibleSize.width - pad, origin.y + visibleSize.height - pad));
        this->addChild(_hudTimeLabel, 3);
    }

    if (!_energyNode) {
        _energyNode = Node::create();
        float pad = 10.0f;
        _energyNode->setPosition(Vec2(origin.x + visibleSize.width - pad, origin.y + pad));
        this->addChild(_energyNode, 3);

        float bw = 160.0f, bh = 18.0f;
        auto bg = DrawNode::create();
        Vec2 bl(-bw, 0), br(0, 0), tr(0, bh), tl(-bw, bh);
        Vec2 rect[4] = { bl, br, tr, tl };
        bg->drawSolidPoly(rect, 4, Color4F(0.f,0.f,0.f,0.35f));
        bg->drawLine(bl, br, Color4F(1,1,1,0.5f));
        bg->drawLine(br, tr, Color4F(1,1,1,0.5f));
        bg->drawLine(tr, tl, Color4F(1,1,1,0.5f));
        bg->drawLine(tl, bl, Color4F(1,1,1,0.5f));
        _energyNode->addChild(bg);

        _energyFill = DrawNode::create();
        _energyNode->addChild(_energyFill);

        _energyLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 16);
        _energyLabel->setAnchorPoint(Vec2(1,0.5f));
        _energyLabel->setPosition(Vec2(-4.0f, bh * 0.5f));
        _energyLabel->setColor(Color3B::WHITE);
        _energyNode->addChild(_energyLabel);
    }

    refreshHUD();
}

void RoomScene::refreshHUD() {
    auto &ws = Game::globalState();
    auto seasonName = [](int idx){
        switch (idx % 4) { case 0: return "Spring"; case 1: return "Summer"; case 2: return "Fall"; default: return "Winter"; }
    };
    if (_hudTimeLabel) {
        _hudTimeLabel->setString(StringUtils::format("%s Day %d, %02d:%02d", seasonName(ws.seasonIndex), ws.dayOfSeason, ws.timeHour, ws.timeMinute));
    }
    if (_energyFill && _energyNode) {
        _energyFill->clear();
        float bw = 160.0f, bh = 18.0f;
        float ratio = ws.maxEnergy > 0 ? (static_cast<float>(ws.energy) / static_cast<float>(ws.maxEnergy)) : 0.f;
        ratio = std::max(0.f, std::min(1.f, ratio));
        float fillW = bw * ratio;
        Vec2 bl(-bw, 0), br(-bw + fillW, 0), tr(-bw + fillW, bh), tl(-bw, bh);
        Vec2 rect[4] = { bl, br, tr, tl };
        _energyFill->drawSolidPoly(rect, 4, Color4F(0.2f, 0.8f, 0.25f, 0.85f));
    }
    if (_energyLabel) {
        _energyLabel->setString(StringUtils::format("Energy %d/%d", ws.energy, ws.maxEnergy));
    }
}

void RoomScene::checkBedRegion() {
    if (!_player) return;
    Vec2 p = _player->getPosition();
    _nearBed = _bedRect.containsPoint(p);
    if (_bedPrompt) {
        _bedPrompt->setVisible(_nearBed);
        if (_nearBed) {
            _bedPrompt->setString("Press Space to Sleep");
            _bedPrompt->setPosition(p + Vec2(0, 26));
        }
    }
}

void RoomScene::buildHotbarUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    _hotbarNode = Node::create();
    _hotbarNode->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                  origin.y + 28));
    this->addChild(_hotbarNode, 2);

    int slots = static_cast<int>(_inventory ? _inventory->size() : 0);
    float slotW = 80.0f;
    float slotH = 32.0f;
    float padding = 6.0f;
    float totalWidth = slots * slotW + (slots - 1) * padding;

    // 背景
    auto bg = DrawNode::create();
    Vec2 bgVerts[4] = { Vec2(-totalWidth/2 - 10, -slotH/2 - 8),
                        Vec2( totalWidth/2 + 10, -slotH/2 - 8),
                        Vec2( totalWidth/2 + 10,  slotH/2 + 8),
                        Vec2(-totalWidth/2 - 10,  slotH/2 + 8) };
    bg->drawSolidPoly(bgVerts, 4, Color4F(0.f, 0.f, 0.f, 0.35f));
    _hotbarNode->addChild(bg);

    _hotbarLabels.clear();
    for (int i = 0; i < slots; ++i) {
        float x = -totalWidth/2 + i * (slotW + padding) + slotW/2;
        auto rect = DrawNode::create();
        Vec2 r[4] = { Vec2(x - slotW/2, -slotH/2),
                      Vec2(x + slotW/2, -slotH/2),
                      Vec2(x + slotW/2,  slotH/2),
                      Vec2(x - slotW/2,  slotH/2) };
        rect->drawSolidPoly(r, 4, Color4F(0.15f, 0.15f, 0.15f, 0.6f));
        _hotbarNode->addChild(rect);
        std::string text = "-";
        if (_inventory) {
            if (auto t = _inventory->toolAt(i)) {
                text = t->name;
            } else if (_inventory->isItem(i)) {
                auto st = _inventory->itemAt(i);
                text = StringUtils::format("%s x%d", Game::itemName(st.type), st.quantity);
            }
        }
        auto label = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 18);
        label->setPosition(Vec2(x, 0));
        _hotbarNode->addChild(label);
        _hotbarLabels.push_back(label);
    }

    _hotbarHighlight = DrawNode::create();
    _hotbarNode->addChild(_hotbarHighlight);
    refreshHotbarUI();
}

void RoomScene::refreshHotbarUI() {
    if (!_hotbarNode || !_hotbarHighlight || !_inventory) return;

    int slots = static_cast<int>(_inventory->size());
    if (slots <= 0) return;

    float slotW = 80.0f;
    float slotH = 32.0f;
    float padding = 6.0f;
    float totalWidth = slots * slotW + (slots - 1) * padding;
    int sel = _inventory->selectedIndex();
    float x = -totalWidth/2 + sel * (slotW + padding) + slotW/2;

    _hotbarHighlight->clear();
    Vec2 a(x - slotW/2, -slotH/2);
    Vec2 b(x + slotW/2, -slotH/2);
    Vec2 c(x + slotW/2,  slotH/2);
    Vec2 d(x - slotW/2,  slotH/2);
    _hotbarHighlight->drawLine(a, b, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _hotbarHighlight->drawLine(b, c, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _hotbarHighlight->drawLine(c, d, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _hotbarHighlight->drawLine(d, a, Color4F(1.f, 0.9f, 0.2f, 1.f));

    for (int i = 0; i < slots && i < static_cast<int>(_hotbarLabels.size()); ++i) {
        std::string text = "-";
        if (auto t = _inventory->toolAt(i)) {
            text = t->name;
        } else if (_inventory->isItem(i)) {
            auto st = _inventory->itemAt(i);
            text = StringUtils::format("%s x%d", Game::itemName(st.type), st.quantity);
        }
        _hotbarLabels[i]->setString(text);
    }
}

// 室内箱子绘制：用矩形表示
void RoomScene::refreshChestsVisuals() {
    if (!_chestDraw) return;
    _chestDraw->clear();
    for (const auto& ch : _houseChests) {
        Vec2 center = ch.pos;
        Vec2 a(center.x - 20, center.y - 20);
        Vec2 b(center.x + 20, center.y - 20);
        Vec2 c(center.x + 20, center.y + 20);
        Vec2 d(center.x - 20, center.y + 20);
        Vec2 rect[4] = { a,b,c,d };
        _chestDraw->drawSolidPoly(rect, 4, Color4F(0.85f,0.65f,0.20f,1.0f));
        _chestDraw->drawLine(a,b, Color4F(0,0,0,0.35f));
        _chestDraw->drawLine(b,c, Color4F(0,0,0,0.35f));
        _chestDraw->drawLine(c,d, Color4F(0,0,0,0.35f));
        _chestDraw->drawLine(d,a, Color4F(0,0,0,0.35f));
    }
}

void RoomScene::checkChestRegion() {
    if (!_player) return;
    Vec2 p = _player->getPosition();
    bool isNear = false; // 避免 Win32 头文件中 near/far 宏造成冲突
    float maxDist = 28.0f;
    for (const auto& ch : _houseChests) {
        if (p.distance(ch.pos) <= maxDist) { isNear = true; break; }
    }
    _nearChest = isNear;
    if (_chestPrompt) {
        _chestPrompt->setVisible(_nearChest);
        if (_nearChest) {
            _chestPrompt->setString("Right-click to Open / Space to Deposit");
            _chestPrompt->setPosition(p + Vec2(0, 26));
        }
    }
}

// ---- 室内箱子面板 ----
void RoomScene::buildChestUI() {
    if (_chestPanel) return;
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _chestPanel = Node::create();
    _chestPanel->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
    this->addChild(_chestPanel, 3);
    _chestPanel->setVisible(false);

    // 背景
    auto bg = DrawNode::create();
    float w = _chestPanelW, h = _chestPanelH;
    Vec2 v[4] = { Vec2(-w/2,-h/2), Vec2(w/2,-h/2), Vec2(w/2,h/2), Vec2(-w/2,h/2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f,0.f,0.f,0.55f));
    _chestPanel->addChild(bg);

    auto title = Label::createWithTTF("Chest Storage", "fonts/Marker Felt.ttf", 20);
    title->setPosition(Vec2(0, h/2 - 26));
    _chestPanel->addChild(title);

    // 关闭按钮
    auto closeBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(18);
    closeBtn->setScale9Enabled(true);
    closeBtn->setContentSize(Size(36, 36));
    closeBtn->setPosition(Vec2(w/2 - 20, h/2 - 20));
    closeBtn->addClickEventListener([this](Ref*){
        if (_chestPanel) _chestPanel->setVisible(false);
    });
    _chestPanel->addChild(closeBtn);
    // 行容器：用于动态生成/清空条目
    _chestListNode = Node::create();
    _chestPanel->addChild(_chestListNode);
}

void RoomScene::refreshChestUI() {
    if (!_chestPanel || _activeChestIdx < 0 || _activeChestIdx >= (int)_houseChests.size()) return;
    auto &bag = _houseChests[_activeChestIdx].bag;
    if (_chestListNode) _chestListNode->removeAllChildren();
    _withdrawRows.clear();
    const float startY = _chestRowStartY;
    const float gapY = _chestRowGapY;
    int i = 0;
    for (const auto &entry : bag.all()) {
        auto t = entry.first; int have = entry.second;
        if (have <= 0) continue;
        float y = startY - i * gapY;
        auto nameLabel = Label::createWithTTF(Game::itemName(t), "fonts/Marker Felt.ttf", 18);
        nameLabel->setPosition(Vec2(-140, y));
        if (_chestListNode) _chestListNode->addChild(nameLabel);
        auto countLabel = Label::createWithTTF(StringUtils::format("x%d", have), "fonts/Marker Felt.ttf", 18);
        countLabel->setPosition(Vec2(-60, y));
        if (_chestListNode) _chestListNode->addChild(countLabel);
        WithdrawRow row; row.type = t; row.nameLabel = nameLabel; row.countLabel = countLabel; row.planLabel = nullptr; row.minusBtn = nullptr; row.plusBtn = nullptr; row.takeBtn = nullptr; row.planQty = 0;
        _withdrawRows.push_back(row);
        ++i;
    }
}

void RoomScene::showChestPanel(int idx) {
    _activeChestIdx = idx;
    if (_chestPanel) {
        _chestPanel->setVisible(true);
        refreshChestUI();
    }
}

void RoomScene::attemptWithdraw(Game::ItemType type, int qty) {
    if (_activeChestIdx < 0 || _activeChestIdx >= (int)_houseChests.size()) return;
    if (qty <= 0) return;
    auto &bag = _houseChests[_activeChestIdx].bag;
    int have = bag.count(type);
    int take = std::min(have, qty);
    if (take <= 0) {
        auto warn = Label::createWithTTF("Empty", "fonts/Marker Felt.ttf", 20);
        warn->setColor(Color3B::RED);
        Vec2 pos = _houseChests[_activeChestIdx].pos;
        warn->setPosition(pos + Vec2(0, 26));
        this->addChild(warn, 3);
        auto seq = Sequence::create(FadeOut::create(0.6f), RemoveSelf::create(), nullptr);
        warn->runAction(seq);
        return;
    }

    bag.remove(type, take);
    int rem = _inventory ? _inventory->addItems(type, take) : take;
    int put = take - rem;
    Vec2 chestPos = _houseChests[_activeChestIdx].pos;
    if (rem > 0) {
        // 背包装不下的部分退回箱子
        bag.add(type, rem);
        auto warn = Label::createWithTTF(StringUtils::format("Inventory full, took %d", put), "fonts/Marker Felt.ttf", 20);
        warn->setColor(Color3B::RED);
        warn->setPosition(chestPos + Vec2(0, 26));
        this->addChild(warn, 3);
        auto seq = Sequence::create(FadeOut::create(0.6f), RemoveSelf::create(), nullptr);
        warn->runAction(seq);
    } else {
        auto pop = Label::createWithTTF(StringUtils::format("Took %d", put), "fonts/Marker Felt.ttf", 20);
        pop->setColor(Color3B::YELLOW);
        pop->setPosition(chestPos + Vec2(0, 26));
        this->addChild(pop, 3);
        auto seq = Sequence::create(FadeOut::create(0.6f), RemoveSelf::create(), nullptr);
        pop->runAction(seq);
    }

    // 同步全局与 UI
    Game::globalState().houseChests = _houseChests;
    refreshChestUI();
    refreshHotbarUI();
}