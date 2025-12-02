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
            case EventKeyboard::KeyCode::KEY_Q:          if (_inventory) { _inventory->prev(); Game::globalState().selectedIndex = _inventory->selectedIndex(); refreshHotbarUI(); } break;
            case EventKeyboard::KeyCode::KEY_E:          if (_inventory) { _inventory->next(); Game::globalState().selectedIndex = _inventory->selectedIndex(); refreshHotbarUI(); } break;
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