#include "Controllers/FarmMapController.h"
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

    _gameMap = Game::GameMap::create("Maps/spring_outdoors/spring_outdoors.tmx");
    Size content = _gameMap ? _gameMap->getContentSize() : Size(_cols * GameConfig::TILE_SIZE, _rows * GameConfig::TILE_SIZE);
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _mapOrigin = Vec2(origin.x + (visibleSize.width - content.width) * 0.5f,
                      origin.y + (visibleSize.height - content.height) * 0.5f);
    if (_gameMap) {
        _gameMap->setAnchorPoint(Vec2(0,0));
        _gameMap->setPosition(_mapOrigin);
        _mapNode->addChild(_gameMap, 0);
        _cols = static_cast<int>(_gameMap->getMapSize().width);
        _rows = static_cast<int>(_gameMap->getMapSize().height);
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
        auto placeIfSoil = [this, &safe](int c, int r, Game::TileType t){ if (inBounds(c,r) && !safe(c,r) && getTile(c,r) == Game::TileType::Soil) setTile(c,r,t); };
        int rocks = (_cols * _rows) / 18;
        int trees = (_cols * _rows) / 14;
        for (int i = 0; i < rocks; ++i) { placeIfSoil(distC(rng), distR(rng), Game::TileType::Rock); }
        for (int i = 0; i < trees; ++i) { placeIfSoil(distC(rng), distR(rng), Game::TileType::Tree); }
        ws.farmTiles = _tiles;
    } else {
        // 同步已有瓦片
        _tiles = ws.farmTiles;
    }

    _mapDraw = DrawNode::create();
    _mapNode->addChild(_mapDraw, -1);

    _cursor = DrawNode::create();
    if (_gameMap && _gameMap->getTMX()) {
        _gameMap->getTMX()->addChild(_cursor, 21);
    } else {
        _mapNode->addChild(_cursor, 1);
    }

    // Drops / Chests / Crops 从全局恢复
    _drops = ws.farmDrops;
    _chests = ws.farmChests;
    _crops = ws.farmCrops;
    _dropsDraw = DrawNode::create();
    if (_gameMap && _gameMap->getTMX()) {
        _gameMap->getTMX()->addChild(_dropsDraw, 19);
    } else {
        _worldNode->addChild(_dropsDraw, 1);
    }
    _chestDraw = DrawNode::create();
    _worldNode->addChild(_chestDraw, 1);
    _cropsDraw = DrawNode::create();
    _worldNode->addChild(_cropsDraw, 1);
    _cropsRoot = Node::create();
    if (_gameMap && _gameMap->getTMX()) {
        _gameMap->getTMX()->addChild(_cropsRoot, 22);
    } else {
        _worldNode->addChild(_cropsRoot, 18);
    }

    // Tile overlay root for textured tilled soil
    _tileRoot = Node::create();
    if (_gameMap && _gameMap->getTMX()) {
        _gameMap->getTMX()->addChild(_tileRoot, 18);
    } else {
        _worldNode->addChild(_tileRoot, 0);
    }

    // Lake overlay: generate a natural elliptical patch with configured tile
    _lakeRoot = Node::create();
    if (_gameMap && _gameMap->getTMX()) {
        // 提高层级，确保湖面贴图在基础地表层之上
        _gameMap->getTMX()->addChild(_lakeRoot, 20);
    } else {
        _worldNode->addChild(_lakeRoot, 0);
    }
    {
        // Choose a location and shape for the lake
        int cx = std::max(8, _cols / 4);
        int cy = std::max(8, _rows / 3);
        int a = 8; // horizontal radius (tiles)
        int b = 6; // vertical radius (tiles)
        for (int r = std::max(0, cy - b - 1); r <= std::min(_rows - 1, cy + b + 1); ++r) {
            for (int c = std::max(0, cx - a - 1); c <= std::min(_cols - 1, cx + a + 1); ++c) {
                float dx = static_cast<float>(c - cx);
                float dy = static_cast<float>(r - cy);
                float v = (dx*dx) / (static_cast<float>(a*a)) + (dy*dy) / (static_cast<float>(b*b));
                if (v <= 1.0f) {
                    // Ensure no rocks/trees remain inside the lake
                    if (inBounds(c,r)) {
                        if (getTile(c,r) == Game::TileType::Rock || getTile(c,r) == Game::TileType::Tree) {
                            setTile(c,r, Game::TileType::Soil);
                        }
                    }
                    long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
                    cocos2d::Sprite* spr = cocos2d::Sprite::create("Maps/spring_outdoors/spring_outdoors.png");
                    spr->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
                    // Compute texture rect for lake tile
                    int tw = 16, th = 16;
                    int columns = GameConfig::SPRING_OUTDOORS_COLUMNS;
                    int row1 = GameConfig::FARM_LAKE_TILE_ROW;
                    int col1 = GameConfig::FARM_LAKE_TILE_COL;
                    int tileId = (row1 - 1) * columns + (col1 - 1);
                    int col = tileId % columns;
                    int row = tileId / columns;
                    float texH = spr->getTexture() ? spr->getTexture()->getContentSize().height : 0.0f;
                    float x = static_cast<float>(col * tw);
                    float y = texH - static_cast<float>((row + 1) * th);
                    spr->setTextureRect(cocos2d::Rect(x, y, static_cast<float>(tw), static_cast<float>(th)));
                    auto pos = tileToWorld(c, r);
                    spr->setPosition(pos);
                    _lakeRoot->addChild(spr, 0);
                    _lakeSprites[key] = spr;
                }
            }
        }
    }

    refreshMapVisuals();
    refreshDropsVisuals();
    refreshCropsVisuals();

    // 门口区域
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float doorW = s * 2.0f;
    float doorH = s * 0.75f;
    float dx = (_cols * s) * 0.5f - doorW * 0.5f;
    float dy = s * 0.5f - doorH * 0.5f;
    _farmDoorRect = Rect(dx, dy, doorW, doorH);
}

Size FarmMapController::getContentSize() const {
    if (_gameMap) return _gameMap->getContentSize();
    return Size(_cols * GameConfig::TILE_SIZE, _rows * GameConfig::TILE_SIZE);
}

Vec2 FarmMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float mapW = _gameMap ? _gameMap->getContentSize().width : (_cols * s);
    float mapH = _gameMap ? _gameMap->getContentSize().height : (_rows * s);
    Vec2 candidate = next;
    float minX = _gameMap ? (s * 0.5f) : (_mapOrigin.x + s * 0.5f);
    float minY = _gameMap ? (s * 0.5f) : (_mapOrigin.y + s * 0.5f);
    float maxX = _gameMap ? (mapW - s * 0.5f) : (_mapOrigin.x + mapW - s * 0.5f);
    float maxY = _gameMap ? (mapH - s * 0.5f) : (_mapOrigin.y + mapH - s * 0.5f);
    candidate.x = std::max(minX, std::min(maxX, candidate.x));
    candidate.y = std::max(minY, std::min(maxY, candidate.y));

    auto lakeBlocked = [&](const Vec2& p){
        int cc=0, rr=0; worldToTileIndex(p, cc, rr);
        if (!inBounds(cc, rr)) return false;
        long long k = (static_cast<long long>(rr) << 32) | static_cast<unsigned long long>(cc);
        return _lakeSprites.find(k) != _lakeSprites.end();
    };

    Vec2 tryX(candidate.x, current.y);
    if (_gameMap) {
        bool baseBlockedX = _gameMap->collides(tryX, radius);
        bool lakeX = lakeBlocked(tryX) || lakeBlocked(tryX + Vec2(radius,0)) || lakeBlocked(tryX + Vec2(-radius,0));
        if (baseBlockedX || lakeX) {
            tryX.x = current.x;
        }
    }
    Vec2 tryY(current.x, candidate.y);
    if (_gameMap) {
        bool baseBlockedY = _gameMap->collides(tryY, radius);
        bool lakeY = lakeBlocked(tryY) || lakeBlocked(tryY + Vec2(0,radius)) || lakeBlocked(tryY + Vec2(0,-radius));
        if (baseBlockedY || lakeY) {
            tryY.y = current.y;
        }
    }
    return Vec2(tryX.x, tryY.y);
}

bool FarmMapController::collides(const Vec2& pos, float radius) const {
    bool blocked = _gameMap ? _gameMap->collides(pos, radius) : false;
    if (blocked) return true;
    // 追加：湖面不可行走（根据半径采样多点格子判定）
    auto isLakeAt = [&](const Vec2& p){
        int cc = 0, rr = 0; worldToTileIndex(p, cc, rr);
        if (!inBounds(cc, rr)) return false;
        long long k = (static_cast<long long>(rr) << 32) | static_cast<unsigned long long>(cc);
        return _lakeSprites.find(k) != _lakeSprites.end();
    };
    // 采样中心与四个方向（半径）
    if (isLakeAt(pos)) return true;
    Vec2 offsets[4] = {
        Vec2(radius, 0), Vec2(-radius, 0), Vec2(0, radius), Vec2(0, -radius)
    };
    for (auto& d : offsets) {
        if (isLakeAt(pos + d)) return true;
    }
    return false;
}

bool FarmMapController::isNearDoor(const Vec2& playerWorldPos) const {
    return _farmDoorRect.containsPoint(playerWorldPos);
}

bool FarmMapController::isNearChest(const Vec2& playerWorldPos) const {
    float maxDist = GameConfig::TILE_SIZE * 0.8f;
    for (const auto& ch : _chests) {
        if (playerWorldPos.distance(ch.pos) <= maxDist) return true;
    }
    return false;
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

bool FarmMapController::isNearLake(const Vec2& playerWorldPos, float radius) const {
    if (_lakeSprites.empty()) return false;
    float s = tileSize();
    float half = s * 0.5f;
    for (const auto& kv : _lakeSprites) {
        long long key = kv.first;
        int r = static_cast<int>(key >> 32);
        int c = static_cast<int>(key & 0xffffffff);
        auto center = tileToWorld(c, r);
        // 与湖面格子的矩形边界的最近距离（distance to AABB）
        float dx = std::max(std::abs(playerWorldPos.x - center.x) - half, 0.0f);
        float dy = std::max(std::abs(playerWorldPos.y - center.y) - half, 0.0f);
        float distEdge = std::sqrt(dx*dx + dy*dy);
        if (distEdge <= radius) return true;
    }
    return false;
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
    if (_gameMap) return _gameMap->tileToWorld(c, r);
    float s = tileSize();
    return _mapOrigin + Vec2(c * s + s * 0.5f, r * s + s * 0.5f);
}

void FarmMapController::worldToTileIndex(const Vec2& p, int& c, int& r) const {
    if (_gameMap) { _gameMap->worldToTileIndex(p, c, r); return; }
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
                // 去掉浇水提示的蓝色覆盖层
                case Game::TileType::Rock:    _mapDraw->drawSolidCircle(center, s*0.35f, 0.0f, 12, Color4F(0.6f,0.6f,0.6f,1.0f)); break;
                case Game::TileType::Tree:    _mapDraw->drawSolidCircle(center, s*0.45f, 0.0f, 12, Color4F(0.2f,0.75f,0.25f,1.0f)); break;
                default: break;
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
    if (!_cropsRoot) return;
    float s = tileSize();
    std::unordered_set<long long> alive;
    for (const auto& cp : _crops) {
        long long key = (static_cast<long long>(cp.r) << 32) | static_cast<unsigned long long>(cp.c);
        alive.insert(key);
        auto itB = _cropSprites.find(key);
        cocos2d::Sprite* sprB = nullptr;
        if (itB == _cropSprites.end()) {
            sprB = cocos2d::Sprite::create("crops/crops.png");
            sprB->setAnchorPoint(cocos2d::Vec2(0.5f, 0.0f));
            _cropsRoot->addChild(sprB, 0);
            _cropSprites[key] = sprB;
        } else {
            sprB = itB->second;
        }
        auto itT = _cropSpritesTop.find(key);
        cocos2d::Sprite* sprT = nullptr;
        if (itT == _cropSpritesTop.end()) {
            sprT = cocos2d::Sprite::create("crops/crops.png");
            sprT->setAnchorPoint(cocos2d::Vec2(0.5f, 0.0f));
            _cropsRoot->addChild(sprT, 1);
            _cropSpritesTop[key] = sprT;
        } else {
            sprT = itT->second;
        }
        float texH = sprB->getTexture() ? sprB->getTexture()->getContentSize().height : 0.0f;
        cocos2d::Rect rectB = Game::cropRectBottomHalf(cp.type, cp.stage, texH);
        cocos2d::Rect rectT = Game::cropRectTopHalf(cp.type, cp.stage, texH);
        sprB->setTextureRect(rectB);
        sprT->setTextureRect(rectT);
        auto center = tileToWorld(cp.c, cp.r);
        sprB->setPosition(cocos2d::Vec2(center.x, center.y - s * 0.5f));
        sprT->setPosition(cocos2d::Vec2(center.x, center.y - s * 0.5f + 16.0f));
        sprB->setVisible(true);
        sprT->setVisible(true);
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

int FarmMapController::findCropIndex(int c, int r) const {
    for (int i = 0; i < static_cast<int>(_crops.size()); ++i) {
        if (_crops[i].c == c && _crops[i].r == r) return i;
    }
    return -1;
}

void FarmMapController::plantCrop(int cropType, int c, int r) {
    Game::Crop cp; cp.c=c; cp.r=r; cp.type=static_cast<Game::CropType>(cropType); cp.stage=0; cp.progress=0; cp.maxStage=Game::cropMaxStage(cp.type);
    _crops.push_back(cp);
    Game::globalState().farmCrops = _crops;
    refreshCropsVisuals();
}

void FarmMapController::advanceCropsDaily() {
    for (auto &cp : _crops) {
        auto t = getTile(cp.c, cp.r);
        bool watered = (t == Game::TileType::Watered);
        if (watered && cp.stage < cp.maxStage) {
            cp.progress += 1;
            auto days = Game::cropStageDays(cp.type);
            int need = (cp.stage >= 0 && cp.stage < static_cast<int>(days.size())) ? days[cp.stage] : 1;
            if (cp.progress >= need) { cp.stage += 1; cp.progress = 0; }
        }
        if (t == Game::TileType::Watered) { setTile(cp.c, cp.r, Game::TileType::Tilled); }
    }
    Game::globalState().farmCrops = _crops;
}

void FarmMapController::harvestCropAt(int c, int r) {
    int idx = findCropIndex(c, r);
    if (idx < 0) return;
    auto cp = _crops[idx];
    if (cp.stage >= cp.maxStage) {
        auto item = Game::produceItemFor(cp.type);
        // 背包逻辑由上层处理，这里仅移除作物并刷新可视
        _crops.erase(_crops.begin() + idx);
        Game::globalState().farmCrops = _crops;
        refreshCropsVisuals();
    }
}

void FarmMapController::instantMatureAllCrops() {
    for (auto &cp : _crops) {
        cp.stage = cp.maxStage;
        cp.progress = 0;
    }
    Game::globalState().farmCrops = _crops;
    refreshCropsVisuals();
}

void FarmMapController::advanceCropOnceAt(int c, int r) {
    int idx = findCropIndex(c, r);
    if (idx < 0) return;
    auto &cp = _crops[idx];
    if (cp.stage < cp.maxStage) {
        cp.stage += 1;
        cp.progress = 0;
        Game::globalState().farmCrops = _crops;
        refreshCropsVisuals();
    }
}

void FarmMapController::addActorToMap(cocos2d::Node* node, int zOrder) {
    if (_gameMap && _gameMap->getTMX()) {
        _gameMap->getTMX()->addChild(node, zOrder);
    } else if (_worldNode) {
        _worldNode->addChild(node, zOrder);
    }
}

} // namespace Controllers
