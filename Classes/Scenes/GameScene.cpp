/**
 * GameScene: Main game scene placeholder.
 */
#include "Scenes/GameScene.h"
#include "cocos2d.h"
#include "Game/Inventory.h"
#include "Game/Tool.h"
#include "Game/GameConfig.h"
#include "Game/Tile.h"
#include "Game/Item.h"
#include "Game/WorldState.h"
#include "Scenes/RoomScene.h"

USING_NS_CC;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // Placeholder label
    auto label = Label::createWithTTF("Game Scene Placeholder", "fonts/Marker Felt.ttf", 30);
    if (label) {
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height * 0.85f));
        this->addChild(label, 1);
    }

    // Map build
    _mapNode = Node::create();
    this->addChild(_mapNode, 0);
    buildMap();

    // Simple placeholder player: a colored square
    _player = DrawNode::create();
    const float size = 16.f;
    Vec2 verts[4] = { Vec2(-size, -size), Vec2(size, -size), Vec2(size, size), Vec2(-size, size) };
    _player->drawSolidPoly(verts, 4, Color4F(0.2f, 0.7f, 0.9f, 1.0f));
    // place player at center tile
    _player->setPosition(tileToWorld(_cols/2, _rows/2));
    this->addChild(_player, 2);

    // Inventory & hotbar（共享全局背包）
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

    _dropsNode = Node::create();
    this->addChild(_dropsNode, 1);
    _dropsDraw = DrawNode::create();
    _dropsNode->addChild(_dropsDraw);
    refreshDropsVisuals();

    // 门口交互提示（初始隐藏）
    _doorPrompt = Label::createWithTTF("Press Space to Enter House", "fonts/Marker Felt.ttf", 20);
    if (_doorPrompt) {
        _doorPrompt->setColor(Color3B::YELLOW);
        _doorPrompt->setVisible(false);
        this->addChild(_doorPrompt, 3);
    }

    // Keyboard movement (placeholder)
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*) {
        switch (code) {
            case EventKeyboard::KeyCode::KEY_W:          _up = true; break;
            case EventKeyboard::KeyCode::KEY_UP_ARROW:   _up = true; break;
            case EventKeyboard::KeyCode::KEY_S:          _down = true; break;
            case EventKeyboard::KeyCode::KEY_DOWN_ARROW: _down = true; break;
            case EventKeyboard::KeyCode::KEY_A:          _left = true; break;
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW: _left = true; break;
            case EventKeyboard::KeyCode::KEY_D:          _right = true; break;
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:_right = true; break;
            // hotbar selection（持久化选中索引）
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
                if (_nearFarmDoor) {
                    auto room = RoomScene::create();
                    // 进入室内时出生在门内侧，保持动线自然
                    room->setSpawnInsideDoor();
                    auto trans = TransitionFade::create(0.6f, room);
                    Director::getInstance()->replaceScene(trans);
                } else {
                    useSelectedTool();
                }
                break;
            default: break;
        }
    };
    listener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) {
        switch (code) {
            case EventKeyboard::KeyCode::KEY_W:          _up = false; break;
            case EventKeyboard::KeyCode::KEY_UP_ARROW:   _up = false; break;
            case EventKeyboard::KeyCode::KEY_S:          _down = false; break;
            case EventKeyboard::KeyCode::KEY_DOWN_ARROW: _down = false; break;
            case EventKeyboard::KeyCode::KEY_A:          _left = false; break;
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW: _left = false; break;
            case EventKeyboard::KeyCode::KEY_D:          _right = false; break;
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:_right = false; break;
            default: break;
        }
        // 如果没有任何方向键被按住，立即重置加速状态
        if (!(_up || _down || _left || _right)) {
            _moveHeldDuration = 0.0f;
            _isSprinting = false;
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    this->scheduleUpdate();

    return true;
}

void GameScene::update(float dt) {
    // sprint timing: holding ANY movement key (WASD/Arrows)
    bool movementHeld = (_up || _down || _left || _right);
    if (movementHeld) {
        _moveHeldDuration += dt;
        _isSprinting = (_moveHeldDuration >= _sprintThreshold);
    } else {
        _moveHeldDuration = 0.0f;
        _isSprinting = false;
    }

    // accumulate direction from pressed keys for diagonal movement
    float dx = 0.0f;
    float dy = 0.0f;
    if (_left)  dx -= 1.0f;
    if (_right) dx += 1.0f;
    if (_down)  dy -= 1.0f;
    if (_up)    dy += 1.0f;

    if (dx == 0.0f && dy == 0.0f) {
        updateCursor();
        collectDropsNearPlayer();
        checkFarmDoorRegion();
        return;
    }

    cocos2d::Vec2 dir(dx, dy);
    dir.normalize(); // keep consistent speed when moving diagonally
    _lastDir = dir; // track facing

    float speed = _isSprinting ? _sprintSpeed : _baseSpeed;
    cocos2d::Vec2 delta = dir * speed * dt;
    auto nextPos = _player->getPosition() + delta;
    // clamp to map bounds
    float mapW = _cols * GameConfig::TILE_SIZE;
    float mapH = _rows * GameConfig::TILE_SIZE;
    auto minX = _mapOrigin.x + GameConfig::TILE_SIZE * 0.5f;
    auto minY = _mapOrigin.y + GameConfig::TILE_SIZE * 0.5f;
    auto maxX = _mapOrigin.x + mapW - GameConfig::TILE_SIZE * 0.5f;
    auto maxY = _mapOrigin.y + mapH - GameConfig::TILE_SIZE * 0.5f;
    nextPos.x = std::max(minX, std::min(maxX, nextPos.x));
    nextPos.y = std::max(minY, std::min(maxY, nextPos.y));
    _player->setPosition(nextPos);

    updateCursor();
    collectDropsNearPlayer();
    checkFarmDoorRegion();
}

void GameScene::buildMap() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    float mapW = _cols * GameConfig::TILE_SIZE;
    float mapH = _rows * GameConfig::TILE_SIZE;
    _mapOrigin = Vec2(origin.x + (visibleSize.width - mapW) * 0.5f,
                      origin.y + (visibleSize.height - mapH) * 0.5f);

    // 地图：从全局状态恢复或首次初始化
    auto &ws = Game::globalState();
    if (ws.farmTiles.empty()) {
        _tiles.assign(_cols * _rows, Game::TileType::Soil);
        // 首次放置演示用岩石与树
        int cx = _cols / 2;
        int cy = _rows / 2;
        auto place = [this](int c, int r, Game::TileType t){ if (inBounds(c,r)) setTile(c,r,t); };
        place(cx-5, cy-2, Game::TileType::Rock);
        place(cx-4, cy,   Game::TileType::Rock);
        place(cx-6, cy+2, Game::TileType::Rock);
        place(cx+4, cy-1, Game::TileType::Tree);
        place(cx+5, cy+1, Game::TileType::Tree);
        place(cx+6, cy,   Game::TileType::Tree);
        ws.farmTiles = _tiles;
    } else {
        _tiles = ws.farmTiles;
    }

    _mapDraw = DrawNode::create();
    _mapNode->addChild(_mapDraw);

    _cursor = DrawNode::create();
    _mapNode->addChild(_cursor, 1);

    refreshMapVisuals();
    updateCursor();

    // 掉落：从全局状态恢复
    _drops = ws.farmDrops;

    // 定义农场房屋门口区域（靠近底部中间）
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float doorW = s * 2.0f;
    float doorH = s * 0.75f;
    float dx = _mapOrigin.x + (_cols * s) * 0.5f - doorW * 0.5f;
    float dy = _mapOrigin.y + s * 0.5f - doorH * 0.5f; // 靠近底边中心
    _farmDoorRect = Rect(dx, dy, doorW, doorH);
}

void GameScene::refreshMapVisuals() {
    if (!_mapDraw) return;
    _mapDraw->clear();

    float s = static_cast<float>(GameConfig::TILE_SIZE);
    for (int r = 0; r < _rows; ++r) {
        for (int c = 0; c < _cols; ++c) {
            auto center = tileToWorld(c, r);
            // rect corners
            Vec2 a(center.x - s/2, center.y - s/2);
            Vec2 b(center.x + s/2, center.y - s/2);
            Vec2 c2(center.x + s/2, center.y + s/2);
            Vec2 d(center.x - s/2, center.y + s/2);

            Color4F base;
            switch (getTile(c, r)) {
                case Game::TileType::Soil:   base = Color4F(0.55f, 0.40f, 0.25f, 1.0f); break;
                case Game::TileType::Tilled: base = Color4F(0.45f, 0.30f, 0.18f, 1.0f); break;
                case Game::TileType::Watered:base = Color4F(0.40f, 0.28f, 0.16f, 1.0f); break; // darker soil as base
                case Game::TileType::Rock:   base = Color4F(0.55f, 0.40f, 0.25f, 1.0f); break;
                case Game::TileType::Tree:   base = Color4F(0.55f, 0.40f, 0.25f, 1.0f); break;
            }
            Vec2 rect[4] = { a, b, c2, d };
            _mapDraw->drawSolidPoly(rect, 4, base);
            _mapDraw->drawLine(a,b, Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(b,c2,Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(c2,d,Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(d,a, Color4F(0,0,0,0.25f));

            // overlays for special tiles
            switch (getTile(c, r)) {
                case Game::TileType::Watered:
                    _mapDraw->drawSolidPoly(rect, 4, Color4F(0.2f, 0.4f, 0.9f, 0.22f));
                    break;
                case Game::TileType::Rock:
                    _mapDraw->drawSolidCircle(center, s*0.35f, 0.0f, 12, Color4F(0.6f,0.6f,0.6f,1.0f));
                    break;
                case Game::TileType::Tree:
                    _mapDraw->drawSolidCircle(center, s*0.45f, 0.0f, 12, Color4F(0.2f,0.75f,0.25f,1.0f));
                    break;
                default: break;
            }
        }
    }
}

void GameScene::updateCursor() {
    if (!_cursor) return;
    _cursor->clear();
    auto target = targetTile();
    int tc = target.first;
    int tr = target.second;
    if (!inBounds(tc,tr)) return;
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    auto center = tileToWorld(tc, tr);
    Vec2 a(center.x - s/2, center.y - s/2);
    Vec2 b(center.x + s/2, center.y - s/2);
    Vec2 c(center.x + s/2, center.y + s/2);
    Vec2 d(center.x - s/2, center.y + s/2);
    _cursor->drawLine(a,b, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _cursor->drawLine(b,c, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _cursor->drawLine(c,d, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _cursor->drawLine(d,a, Color4F(1.f, 0.9f, 0.2f, 1.f));
}

bool GameScene::inBounds(int c, int r) const {
    return c >= 0 && r >= 0 && c < _cols && r < _rows;
}

Game::TileType GameScene::getTile(int c, int r) const {
    return _tiles[r * _cols + c];
}

void GameScene::setTile(int c, int r, Game::TileType t) {
    _tiles[r * _cols + c] = t;
    Game::globalState().farmTiles = _tiles;
}

cocos2d::Vec2 GameScene::tileToWorld(int c, int r) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return _mapOrigin + Vec2(c * s + s * 0.5f, r * s + s * 0.5f);
}

void GameScene::worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    c = static_cast<int>((p.x - _mapOrigin.x) / s);
    r = static_cast<int>((p.y - _mapOrigin.y) / s);
}

std::pair<int,int> GameScene::targetTile() const {
    int pc, pr;
    worldToTileIndex(_player ? _player->getPosition() : Vec2(), pc, pr);
    int dc = (_lastDir.x > 0.1f) ? 1 : ((_lastDir.x < -0.1f) ? -1 : 0);
    int dr = (_lastDir.y > 0.1f) ? 1 : ((_lastDir.y < -0.1f) ? -1 : 0);
    int tc = pc + dc;
    int tr = pr + dr;
    // if not moving, default to facing down
    if (dc == 0 && dr == 0) { tr = pr - 1; }
    // clamp
    if (tc < 0) tc = 0; if (tc >= _cols) tc = _cols-1;
    if (tr < 0) tr = 0; if (tr >= _rows) tr = _rows-1;
    return {tc, tr};
}

void GameScene::buildHotbarUI() {
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

    // background
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

void GameScene::refreshHotbarUI() {
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

    // 更新每个槽位文本（工具名或物品堆叠数量）
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

void GameScene::useSelectedTool() {
    if (!_inventory) return;
    // 如果选中工具则使用，否则非可使用物品不响应
    const Game::Tool* tool = _inventory->selectedTool();
    if (!tool) return;

    std::string msg;
    auto tgt = targetTile();
    int tc = tgt.first, tr = tgt.second;
    if (!inBounds(tc, tr)) return;
    auto current = getTile(tc, tr);

    switch (tool->type) {
        case Game::ToolType::Hoe:
            if (current == Game::TileType::Soil) { 
                setTile(tc,tr, Game::TileType::Tilled); 
                msg = "Till!"; 
                // tilling土壤，掉落纤维
                spawnDropAt(tc, tr, Game::ItemType::Fiber, 1);
            }
            else msg = "Nothing";
            break;
        case Game::ToolType::WateringCan:
            if (current == Game::TileType::Tilled) { setTile(tc,tr, Game::TileType::Watered); msg = "Water!"; }
            else msg = "Nothing";
            break;
        case Game::ToolType::Pickaxe:
            if (current == Game::TileType::Rock) { 
                setTile(tc,tr, Game::TileType::Soil); 
                msg = "Mine!"; 
                // 采矿掉落石头
                spawnDropAt(tc, tr, Game::ItemType::Stone, 1);
            }
            else msg = "Nothing";
            break;
        case Game::ToolType::Axe:
            if (current == Game::TileType::Tree) { 
                setTile(tc,tr, Game::TileType::Soil); 
                msg = "Chop!"; 
                // 砍树掉落木材
                spawnDropAt(tc, tr, Game::ItemType::Wood, 1);
            }
            else msg = "Nothing";
            break;
        default: msg = "Use"; break;
    }

    refreshMapVisuals();
    refreshDropsVisuals();

    auto pop = Label::createWithTTF(msg, "fonts/Marker Felt.ttf", 20);
    pop->setColor(Color3B::YELLOW);
    auto pos = _player ? _player->getPosition() : Vec2(0,0);
    pop->setPosition(pos + Vec2(0, 26));
    this->addChild(pop, 3);
    auto seq = Sequence::create(FadeOut::create(0.6f), RemoveSelf::create(), nullptr);
    pop->runAction(seq);
}

// 掉落与物品：渲染与拾取
void GameScene::refreshDropsVisuals() {
    if (!_dropsDraw) return;
    _dropsDraw->clear();
    for (const auto& d : _drops) {
        _dropsDraw->drawSolidCircle(d.pos, GameConfig::DROP_DRAW_RADIUS, 0.0f, 12, Game::itemColor(d.type));
        // 简单外框
        _dropsDraw->drawCircle(d.pos, GameConfig::DROP_DRAW_RADIUS, 0.0f, 12, false, Color4F(0,0,0,0.4f));
    }
}

void GameScene::spawnDropAt(int c, int r, Game::ItemType type, int qty) {
    if (!inBounds(c,r) || qty <= 0) return;
    Game::Drop d{ type, tileToWorld(c,r), qty };
    _drops.push_back(d);
    Game::globalState().farmDrops = _drops;
}

void GameScene::collectDropsNearPlayer() {
    if (!_player || !_inventory) return;
    const Vec2 p = _player->getPosition();
    bool changed = false;
    auto it = _drops.begin();
    while (it != _drops.end()) {
        float dist = p.distance(it->pos);
        if (dist <= GameConfig::DROP_PICK_RADIUS) {
            int remaining = _inventory->addItems(it->type, it->qty);
            if (remaining <= 0) {
                it = _drops.erase(it);
            } else {
                it->qty = remaining;
                ++it;
            }
            changed = true;
        } else {
            ++it;
        }
    }
    if (changed) {
        refreshDropsVisuals();
        refreshHotbarUI();
        Game::globalState().farmDrops = _drops;
    }
}

// 统一背包后，物品数量显示内嵌到热键栏文本中，已移除独立物品UI。

void GameScene::checkFarmDoorRegion() {
    if (!_player) return;
    Vec2 p = _player->getPosition();
    _nearFarmDoor = _farmDoorRect.containsPoint(p);
    if (_doorPrompt) {
        _doorPrompt->setVisible(_nearFarmDoor);
        if (_nearFarmDoor) {
            _doorPrompt->setString("Press Space to Enter House");
            _doorPrompt->setPosition(p + Vec2(0, 26));
        }
    }
}

void GameScene::setSpawnAtFarmEntrance() {
    if (!_player) return;
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    Vec2 spawn(_farmDoorRect.getMidX(), _farmDoorRect.getMinY() + s);
    _player->setPosition(spawn);
}