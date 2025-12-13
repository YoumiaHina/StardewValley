#include "Controllers/Map/FarmMapController.h"
#include "cocos2d.h"
#include <random>
#include <ctime>
#include <algorithm>
#include <unordered_set>
#include "Game/GameConfig.h"
#include "Game/WorldState.h"
#include "Game/CropSprites.h"

using namespace cocos2d;

namespace Controllers {

void FarmMapController::init() {
    if (!_worldNode) return;
    _mapNode = Node::create();
    _worldNode->addChild(_mapNode, 0);

    _farmMap = Game::FarmMap::create("Maps/spring_outdoors/spring_outdoors.tmx");
    Size content = _farmMap ? _farmMap->getContentSize() : Size(_cols * GameConfig::TILE_SIZE, _rows * GameConfig::TILE_SIZE);
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _mapOrigin = Vec2(origin.x + (visibleSize.width - content.width) * 0.5f,
                      origin.y + (visibleSize.height - content.height) * 0.5f);
    if (_farmMap) {
        _farmMap->setAnchorPoint(Vec2(0,0));
        _farmMap->setPosition(_mapOrigin);
        _mapNode->addChild(_farmMap, 0);
        _cols = static_cast<int>(_farmMap->getMapSize().width);
        _rows = static_cast<int>(_farmMap->getMapSize().height);
    }

    auto &ws = Game::globalState();
    if (ws.farmTiles.empty()) {
        _tiles.assign(_cols * _rows, Game::TileType::Soil);
        int cx = _cols / 2;
        int cy = _rows / 2;
        auto safe = [cx, cy](int c, int r){ return std::abs(c - cx) <= 4 && std::abs(r - cy) <= 4; };
        std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::uniform_int_distribution<int> distC(0, _cols - 1);
        std::uniform_int_distribution<int> distR(0, _rows - 1);
        auto placeIfSoil = [this, &safe](int c, int r, Game::TileType t){
            if (!inBounds(c,r)) return;
            if (safe(c,r)) return;
            if (getTile(c,r) != Game::TileType::Soil) return;
            if (t == Game::TileType::Tree && _farmMap) {
                auto center = this->tileToWorld(c, r);
                cocos2d::Vec2 footCenter = center + cocos2d::Vec2(0, -static_cast<float>(GameConfig::TILE_SIZE) * 0.5f);
                if (_farmMap->inNoTreeArea(footCenter)) return;
            }
            setTile(c, r, t);
        };
        int rocks = (_cols * _rows) / 22;
        int trees = (_cols * _rows) / 40;
        for (int i = 0; i < rocks; ++i) { placeIfSoil(distC(rng), distR(rng), Game::TileType::Rock); }
        for (int i = 0; i < trees; ++i) { placeIfSoil(distC(rng), distR(rng), Game::TileType::Tree); }
        ws.farmTiles = _tiles;
        ws.farmCols = _cols;
        ws.farmRows = _rows;
    } else {
        // 同步已有瓦片
        _tiles = ws.farmTiles;
        // 如果 WorldState 中未保存尺寸（旧存档兼容），则补全
        if (ws.farmCols == 0) ws.farmCols = _cols;
        if (ws.farmRows == 0) ws.farmRows = _rows;
    }

    _mapDraw = DrawNode::create();
    _mapNode->addChild(_mapDraw, -1);

    _cursor = DrawNode::create();
    if (_farmMap && _farmMap->getTMX()) {
        _farmMap->getTMX()->addChild(_cursor, 21);
    } else {
        _mapNode->addChild(_cursor, 1);
    }

    // Drops / Chests 从全局恢复
    _drops = ws.farmDrops;
    _chests = ws.farmChests;
    _dropsDraw = DrawNode::create();
    if (_farmMap && _farmMap->getTMX()) {
        _farmMap->getTMX()->addChild(_dropsDraw, 19);
    } else {
        _worldNode->addChild(_dropsDraw, 1);
    }
    _chestDraw = DrawNode::create();
    _worldNode->addChild(_chestDraw, 1);
    _cropsDraw = DrawNode::create();
    _worldNode->addChild(_cropsDraw, 1);
    _cropsRoot = Node::create();
    if (_farmMap && _farmMap->getTMX()) {
        _farmMap->getTMX()->addChild(_cropsRoot, 22);
    } else {
        _worldNode->addChild(_cropsRoot, 18);
    }

    // Tile overlay root for textured tilled soil
    _tileRoot = Node::create();
    if (_farmMap && _farmMap->getTMX()) {
        _farmMap->getTMX()->addChild(_tileRoot, 18);
    } else {
        _worldNode->addChild(_tileRoot, 0);
    }

    _actorsRoot = Node::create();
    if (_farmMap && _farmMap->getTMX()) {
        _farmMap->getTMX()->addChild(_actorsRoot, 20);
    } else {
        _worldNode->addChild(_actorsRoot, 20);
    }

    _treeSystem = new Controllers::TreeSystem();
    _treeSystem->attachTo(_actorsRoot);


    refreshMapVisuals();
    refreshDropsVisuals();
    refreshCropsVisuals();

    if (_treeSystem && _treeSystem->isEmpty()) {
        if (!ws.farmTrees.empty()) {
            for (const auto& tp : ws.farmTrees) {
                auto center = tileToWorld(tp.c, tp.r);
                _treeSystem->spawnFromTile(tp.c, tp.r, center, _farmMap, GameConfig::TILE_SIZE);
            }
        } else {
            std::vector<Game::TreePos> saved;
            int created = 0;
            for (int r = 0; r < _rows; ++r) {
                for (int c = 0; c < _cols; ++c) {
                    if (getTile(c, r) == Game::TileType::Tree) {
                        auto center = tileToWorld(c, r);
                        Vec2 footCenter = center + Vec2(0, -static_cast<float>(GameConfig::TILE_SIZE) * 0.5f);
                        bool skip = _farmMap && _farmMap->inNoTreeArea(footCenter);
                        if (skip) continue;
                        if (_treeSystem->spawnFromTile(c, r, center, _farmMap, GameConfig::TILE_SIZE)) {
                            setTile(c, r, Game::TileType::Soil);
                            saved.push_back(Game::TreePos{c, r});
                            created++;
                        }
                    }
                }
            }
            if (created == 0) {
                int cx = _cols / 2;
                int cy = _rows / 2;
                auto safe = [this, cx, cy](int c, int r){
                    if (std::abs(c - cx) <= 4 && std::abs(r - cy) <= 4) return true;
                    if (_farmMap) {
                        auto center = this->tileToWorld(c, r);
                        Vec2 footCenter = center + Vec2(0, -static_cast<float>(GameConfig::TILE_SIZE) * 0.5f);
                        if (_farmMap->inNoTreeArea(footCenter)) return true;
                    }
                    return false;
                };
                int trees = (_cols * _rows) / 40;
                _treeSystem->spawnRandom(trees, _cols, _rows,
                    [this](int c, int r){ return this->tileToWorld(c, r); },
                    _farmMap, GameConfig::TILE_SIZE,
                    [safe](int c, int r){ return safe(c, r); }
                );
                saved = _treeSystem->getAllTreeTiles();
            }
            ws.farmTrees = saved;
        }
    }

    // 门口区域
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float doorW = s * 2.0f;
    float doorH = s * 0.75f;
    float dx = (_cols * s) * 0.5f - doorW * 0.5f;
    float dy = s * 0.5f - doorH * 0.5f;
    _farmDoorRect = Rect(dx, dy, doorW, doorH);
}

Size FarmMapController::getContentSize() const {
    if (_farmMap) return _farmMap->getContentSize();
    return Size(_cols * GameConfig::TILE_SIZE, _rows * GameConfig::TILE_SIZE);
}

Vec2 FarmMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float mapW = _farmMap ? _farmMap->getContentSize().width : (_cols * s);
    float mapH = _farmMap ? _farmMap->getContentSize().height : (_rows * s);
    Vec2 candidate = next;
    float minX = _farmMap ? (s * 0.5f) : (_mapOrigin.x + s * 0.5f);
    float minY = _farmMap ? (s * 0.5f) : (_mapOrigin.y + s * 0.5f);
    float maxX = _farmMap ? (mapW - s * 0.5f) : (_mapOrigin.x + mapW - s * 0.5f);
    float maxY = _farmMap ? (mapH - s * 0.5f) : (_mapOrigin.y + mapH - s * 0.5f);
    candidate.x = std::max(minX, std::min(maxX, candidate.x));
    candidate.y = std::max(minY, std::min(maxY, candidate.y));

    Vec2 tryX(candidate.x, current.y);
    if (_farmMap) {
        Vec2 footX = tryX + Vec2(0, -s * 0.5f);
        bool baseBlockedX = _farmMap->collides(footX, radius);
        bool treeBlockedX = false;
        if (_treeSystem) {
            treeBlockedX = _treeSystem->collides(footX, radius * 0.75f, GameConfig::TILE_SIZE);
        }
        if (baseBlockedX || treeBlockedX) {
            tryX.x = current.x;
        }
    }
    Vec2 tryY(current.x, candidate.y);
    if (_farmMap) {
        Vec2 footY = tryY + Vec2(0, -s * 0.5f);
        bool baseBlockedY = _farmMap->collides(footY, radius);
        bool treeBlockedY = false;
        if (_treeSystem) {
            treeBlockedY = _treeSystem->collides(footY, radius * 0.75f, GameConfig::TILE_SIZE);
        }
        if (baseBlockedY || treeBlockedY) {
            tryY.y = current.y;
        }
    }
    return Vec2(tryX.x, tryY.y);
}

bool FarmMapController::collides(const Vec2& pos, float radius) const {
    if (_farmMap && _farmMap->collides(pos, radius)) return true;
    if (_treeSystem && _treeSystem->collides(pos, radius, GameConfig::TILE_SIZE)) return true;
    return false;
}

bool FarmMapController::isNearDoor(const Vec2& playerWorldPos) const {
    if (_farmMap) {
        return _farmMap->nearDoorToRoom(playerWorldPos);
    }
    return _farmDoorRect.containsPoint(playerWorldPos - _mapOrigin);
}

bool FarmMapController::isNearMineDoor(const Vec2& playerWorldPos) const {
    return _farmMap ? _farmMap->nearDoorToMine(playerWorldPos) : false;
}

bool FarmMapController::isNearBeachDoor(const Vec2& playerWorldPos) const {
    return _farmMap ? _farmMap->nearDoorToBeach(playerWorldPos) : false;
}

bool FarmMapController::isNearTownDoor(const Vec2& playerWorldPos) const {
    return _farmMap ? _farmMap->nearDoorToTown(playerWorldPos) : false;
}

bool FarmMapController::isNearChest(const Vec2& playerWorldPos) const {
    float maxDist = GameConfig::TILE_SIZE * 0.8f;
    for (const auto& ch : _chests) {
        if (playerWorldPos.distance(ch.pos) <= maxDist) return true;
    }
    return false;
}

bool FarmMapController::isNearLake(const Vec2& playerWorldPos, float radius) const {
    return _farmMap ? _farmMap->nearWater(playerWorldPos, radius) : false;
}

void FarmMapController::sortActorWithEnvironment(cocos2d::Node* actor) {
    if (!actor) return;
    float s = tileSize();
    float footY = actor->getPositionY() - s * 0.5f; // player node is at tile center; use foot for sorting
    actor->setLocalZOrder(static_cast<int>(-footY));
    if (_treeSystem) _treeSystem->sortTrees();
}

Game::Tree* FarmMapController::findTreeAt(int c, int r) const {
    return _treeSystem ? _treeSystem->findTreeAt(c, r) : nullptr;
}

bool FarmMapController::damageTreeAt(int c, int r, int amount) {
    if (!_treeSystem) return false;
    bool ok = _treeSystem->damageTreeAt(c, r, amount,
        [this](int c,int r,int item){ this->spawnDropAt(c, r, item, 3); this->refreshDropsVisuals(); },
        [this](int c,int r, Game::TileType t){ this->setTile(c, r, t); }
    );
    if (ok && !_treeSystem->findTreeAt(c, r)) {
        auto& ws = Game::globalState();
        std::vector<Game::TreePos> v;
        v.reserve(ws.farmTrees.size());
        for (const auto& tp : ws.farmTrees) {
            if (!(tp.c == c && tp.r == r)) v.push_back(tp);
        }
        ws.farmTrees = v;
    }
    return ok;
}

cocos2d::Vec2 FarmMapController::farmMineDoorSpawnPos() const {
    if (_farmMap) {
        return _farmMap->doorToMineCenter();
    }
    // fallback：使用农场门矩形中心
    return Vec2(_farmDoorRect.getMidX(), _farmDoorRect.getMidY());
}

cocos2d::Vec2 FarmMapController::farmRoomDoorSpawnPos() const {
    if (_farmMap) {
        return _farmMap->doorToRoomCenter();
    }
    return Vec2(_farmDoorRect.getMidX(), _farmDoorRect.getMidY());
}

cocos2d::Vec2 FarmMapController::farmBeachDoorSpawnPos() const {
    if (_farmMap) {
        return _farmMap->doorToBeachCenter();
    }
    return Vec2(_farmDoorRect.getMidX(), _farmDoorRect.getMidY());
}

cocos2d::Vec2 FarmMapController::farmTownDoorSpawnPos() const {
    if (_farmMap) {
        return _farmMap->doorToTownCenter();
    }
    return Vec2(_farmDoorRect.getMidX(), _farmDoorRect.getMidY());
}

bool FarmMapController::inBounds(int c, int r) const {
    return c >= 0 && r >= 0 && c < _cols && r < _rows;
}

std::pair<int,int> FarmMapController::targetTile(const Vec2& playerPos, const Vec2& lastDir) const {
    int pc, pr;
    worldToTileIndex(playerPos, pc, pr);
    int dc = (lastDir.x > 0.1f) ? 1 : ((lastDir.x < -0.1f) ? -1 : 0);
    int dr = (lastDir.y > 0.1f) ? 1 : ((lastDir.y < -0.1f) ? -1 : 0);
    int tc = pc + dc;
    int tr = pr + dr;
    if (dc == 0 && dr == 0) { tr = pr - 1; }
    if (tc < 0) tc = 0; if (tc >= _cols) tc = _cols-1;
    if (tr < 0) tr = 0; if (tr >= _rows) tr = _rows-1;
    return {tc, tr};
}


void FarmMapController::updateCursor(const Vec2& playerPos, const Vec2& lastDir) {
    if (!_cursor) return;
    _cursor->clear();
    auto target = targetTile(playerPos, lastDir);
    int tc = target.first, tr = target.second;
    if (!inBounds(tc,tr)) return;
    float s = tileSize();
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

Game::TileType FarmMapController::getTile(int c, int r) const {
    auto t = _tiles[r * _cols + c];
    return t;
}

void FarmMapController::setTile(int c, int r, Game::TileType t) {
    _tiles[r * _cols + c] = t;
    Game::globalState().farmTiles = _tiles;
    // Immediately refresh visuals to reflect tile changes
    refreshMapVisuals();
}

Vec2 FarmMapController::tileToWorld(int c, int r) const {
    if (_farmMap) return _farmMap->tileToWorld(c, r);
    float s = tileSize();
    return _mapOrigin + Vec2(c * s + s * 0.5f, r * s + s * 0.5f);
}

void FarmMapController::worldToTileIndex(const Vec2& p, int& c, int& r) const {
    if (_farmMap) { _farmMap->worldToTileIndex(p, c, r); return; }
    float s = tileSize();
    c = static_cast<int>((p.x - _mapOrigin.x) / s);
    r = static_cast<int>((p.y - _mapOrigin.y) / s);
}

void FarmMapController::refreshMapVisuals() {
    if (!_mapDraw) return;
    _mapDraw->clear();
    float s = tileSize();
    // Track which overlay sprites are alive
    std::unordered_set<long long> alive;
    for (int r = 0; r < _rows; ++r) {
        for (int c = 0; c < _cols; ++c) {
            auto center = tileToWorld(c, r);
            Vec2 a(center.x - s/2, center.y - s/2);
            Vec2 b(center.x + s/2, center.y - s/2);
            Vec2 c2(center.x + s/2, center.y + s/2);
            Vec2 d(center.x - s/2, center.y + s/2);
            Color4F base;
            switch (getTile(c, r)) {
                case Game::TileType::Soil:   base = Color4F(0.55f, 0.40f, 0.25f, 1.0f); break;
                case Game::TileType::Tilled: base = Color4F(0.45f, 0.30f, 0.18f, 1.0f); break;
                case Game::TileType::Watered:base = Color4F(0.40f, 0.28f, 0.16f, 1.0f); break;
                case Game::TileType::Rock:   base = Color4F(0.55f, 0.40f, 0.25f, 1.0f); break;
                case Game::TileType::Tree:   base = Color4F(0.55f, 0.40f, 0.25f, 1.0f); break;
            }
            Vec2 rect[4] = { a, b, c2, d };
            // Skip base fill for tilled to allow textured overlay to show clearly
            if (getTile(c, r) != Game::TileType::Tilled) {
                _mapDraw->drawSolidPoly(rect, 4, base);
            }
            _mapDraw->drawLine(a,b, Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(b,c2,Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(c2,d, Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(d,a, Color4F(0,0,0,0.25f));
            switch (getTile(c, r)) {
                case Game::TileType::Rock:    _mapDraw->drawSolidCircle(center, s*0.35f, 0.0f, 12, Color4F(0.6f,0.6f,0.6f,1.0f)); break;
                default: break; // 不再为 Tree 绘制占位，避免出现“木桩”效果
            }

            // Tilled soil textured overlay using spring_outdoors tileset (configurable row/col)
            if (getTile(c, r) == Game::TileType::Tilled || getTile(c, r) == Game::TileType::Watered) {
                long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
                alive.insert(key);
                cocos2d::Sprite* spr = nullptr;
                auto it = _tileSprites.find(key);
                if (it == _tileSprites.end()) {
                    spr = cocos2d::Sprite::create("Maps/spring_outdoors/spring_outdoors.png");
                    spr->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
                    _tileRoot->addChild(spr, 0);
                    _tileSprites[key] = spr;
                } else {
                    spr = it->second;
                }
                // Compute texture rect from config row/col (1-based)
                int tw = 16, th = 16;
                int columns = GameConfig::SPRING_OUTDOORS_COLUMNS;
                int row1 = (getTile(c, r) == Game::TileType::Watered)
                    ? GameConfig::FARM_WATERED_TILE_ROW
                    : GameConfig::FARM_TILLED_TILE_ROW;
                int col1 = (getTile(c, r) == Game::TileType::Watered)
                    ? GameConfig::FARM_WATERED_TILE_COL
                    : GameConfig::FARM_TILLED_TILE_COL;
                int tileId = (row1 - 1) * columns + (col1 - 1);
                int col = tileId % columns;
                int row = tileId / columns;
                float texH = spr->getTexture() ? spr->getTexture()->getContentSize().height : 0.0f;
                float x = static_cast<float>(col * tw);
                float y = texH - static_cast<float>((row + 1) * th);
                spr->setTextureRect(cocos2d::Rect(x, y, static_cast<float>(tw), static_cast<float>(th)));
                auto pos = tileToWorld(c, r);
                spr->setPosition(pos);
                spr->setVisible(true);
            }
        }
    }

    // Remove any overlay sprites that are no longer needed
    std::vector<long long> toRemove;
    for (auto &kv : _tileSprites) {
        if (alive.find(kv.first) == alive.end()) {
            toRemove.push_back(kv.first);
        }
    }
    for (auto k : toRemove) {
        if (_tileSprites.count(k)) {
            auto spr = _tileSprites[k];
            if (spr) spr->removeFromParent();
            _tileSprites.erase(k);
        }
    }
    
}

void FarmMapController::refreshCropsVisuals() {
    if (!_cropsRoot || !_cropsDraw) return;
    float s = tileSize();
    std::unordered_set<long long> alive;
    for (const auto& cp : Game::globalState().farmCrops) {
        long long key = (static_cast<long long>(cp.r) << 32) | static_cast<unsigned long long>(cp.c);
        alive.insert(key);

        cocos2d::Sprite* sprB = nullptr;
        cocos2d::Sprite* sprT = nullptr;

        auto itB = _cropSprites.find(key);
        if (itB == _cropSprites.end()) {
            sprB = cocos2d::Sprite::create("Crops/Crops.png");
            if (sprB) {
                sprB->setAnchorPoint(cocos2d::Vec2(0.5f, 0.0f));
                _cropsRoot->addChild(sprB, 0);
                _cropSprites[key] = sprB;
            }
        } else {
            sprB = itB->second;
        }

        auto itT = _cropSpritesTop.find(key);
        if (itT == _cropSpritesTop.end()) {
            sprT = cocos2d::Sprite::create("Crops/Crops.png");
            if (sprT) {
                sprT->setAnchorPoint(cocos2d::Vec2(0.5f, 0.0f));
                _cropsRoot->addChild(sprT, 1);
                _cropSpritesTop[key] = sprT;
            }
        } else {
            sprT = itT->second;
        }

        auto center = tileToWorld(cp.c, cp.r);
        if (sprB && sprT && sprB->getTexture()) {
            float texH = sprB->getTexture()->getContentSize().height;
            cocos2d::Rect rectB = Game::cropRectBottomHalf(cp.type, cp.stage, texH);
            cocos2d::Rect rectT = Game::cropRectTopHalf(cp.type, cp.stage, texH);
            sprB->setTextureRect(rectB);
            sprT->setTextureRect(rectT);
            sprB->setPosition(cocos2d::Vec2(center.x, center.y - s * 0.5f));
            sprT->setPosition(cocos2d::Vec2(center.x, center.y - s * 0.5f + 16.0f));
            sprB->setVisible(true);
            sprT->setVisible(true);
        } else {
            // Fallback: draw placeholder circle if texture not available
            float radius = s * (0.15f + 0.08f * std::max(0, cp.stage));
            Color4F col(0.95f, 0.85f, 0.35f, 1.0f);
            _cropsDraw->drawSolidCircle(center, radius, 0.0f, 12, col);
            _cropsDraw->drawCircle(center, radius, 0.0f, 12, false, Color4F(0,0,0,0.35f));
            if (cp.stage >= cp.maxStage) {
                _cropsDraw->drawCircle(center, radius + 2.0f, 0.0f, 12, false, Color4F(1.f,0.9f,0.2f,0.8f));
            }
        }
    }

    std::vector<long long> toRemove;
    for (auto& kv : _cropSprites) {
        if (alive.find(kv.first) == alive.end()) {
            toRemove.push_back(kv.first);
        }
    }
    for (auto k : toRemove) {
        if (_cropSprites.count(k)) {
            auto spr = _cropSprites[k];
            if (spr) spr->removeFromParent();
            _cropSprites.erase(k);
        }
        if (_cropSpritesTop.count(k)) {
            auto spr2 = _cropSpritesTop[k];
            if (spr2) spr2->removeFromParent();
            _cropSpritesTop.erase(k);
        }
    }
}

void FarmMapController::refreshDropsVisuals() {
    if (!_dropsDraw) return;
    _dropsDraw->clear();
    for (const auto& d : _drops) {
        if (d.type == Game::ItemType::Wood) {
            continue; // 砍树掉落的木材不再绘制棕色圆形
        }
        _dropsDraw->drawSolidCircle(d.pos, GameConfig::DROP_DRAW_RADIUS, 0.0f, 12, Game::itemColor(d.type));
        _dropsDraw->drawCircle(d.pos, GameConfig::DROP_DRAW_RADIUS, 0.0f, 12, false, Color4F(0,0,0,0.4f));
    }
}

void FarmMapController::spawnDropAt(int c, int r, int itemType, int qty) {
    if (!inBounds(c,r) || qty <= 0) return;
    Game::Drop d{ static_cast<Game::ItemType>(itemType), tileToWorld(c,r), qty };
    _drops.push_back(d);
    Game::globalState().farmDrops = _drops;
}

 

void FarmMapController::addActorToMap(cocos2d::Node* node, int /*zOrder*/) {
    if (_actorsRoot) {
        _actorsRoot->addChild(node, 0);
        return;
    }
    if (_farmMap && _farmMap->getTMX()) {
        _farmMap->getTMX()->addChild(node, 20);
    } else if (_worldNode) {
        _worldNode->addChild(node, 20);
    }
}
// namespace Controllers
}
