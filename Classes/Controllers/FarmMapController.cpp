#include "Controllers/FarmMapController.h"
#include "cocos2d.h"
#include <random>
#include <ctime>
#include <algorithm>
#include "Game/GameConfig.h"
#include "Game/WorldState.h"

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

    Vec2 tryX(candidate.x, current.y);
    if (_gameMap) {
        if (!_gameMap->collides(tryX, radius)) {
            // ok
        } else {
            tryX.x = current.x;
        }
    }
    Vec2 tryY(current.x, candidate.y);
    if (_gameMap) {
        if (!_gameMap->collides(tryY, radius)) {
            // ok
        } else {
            tryY.y = current.y;
        }
    }
    return Vec2(tryX.x, tryY.y);
}

bool FarmMapController::collides(const Vec2& pos, float radius) const {
    return _gameMap ? _gameMap->collides(pos, radius) : false;
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
            _mapDraw->drawSolidPoly(rect, 4, base);
            _mapDraw->drawLine(a,b, Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(b,c2,Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(c2,d, Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(d,a, Color4F(0,0,0,0.25f));
            switch (getTile(c, r)) {
                case Game::TileType::Watered: _mapDraw->drawSolidPoly(rect, 4, Color4F(0.2f, 0.4f, 0.9f, 0.22f)); break;
                case Game::TileType::Rock:    _mapDraw->drawSolidCircle(center, s*0.35f, 0.0f, 12, Color4F(0.6f,0.6f,0.6f,1.0f)); break;
                case Game::TileType::Tree:    _mapDraw->drawSolidCircle(center, s*0.45f, 0.0f, 12, Color4F(0.2f,0.75f,0.25f,1.0f)); break;
                default: break;
            }
        }
    }
}

void FarmMapController::refreshCropsVisuals() {
    if (!_cropsDraw) return;
    _cropsDraw->clear();
    float s = tileSize();
    for (const auto& cp : _crops) {
        auto center = tileToWorld(cp.c, cp.r);
        float radius = s * (0.15f + 0.08f * std::max(0, cp.stage));
        Color4F col(0.95f, 0.85f, 0.35f, 1.0f);
        _cropsDraw->drawSolidCircle(center, radius, 0.0f, 12, col);
        _cropsDraw->drawCircle(center, radius, 0.0f, 12, false, Color4F(0,0,0,0.35f));
        if (cp.stage >= cp.maxStage) {
            _cropsDraw->drawCircle(center, radius + 2.0f, 0.0f, 12, false, Color4F(1.f,0.9f,0.2f,0.8f));
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

} // namespace Controllers