#include "Controllers/AbyssMapController.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Controllers {

cocos2d::Size AbyssMapController::getContentSize() const {
    return Size(_cols * GameConfig::TILE_SIZE, _rows * GameConfig::TILE_SIZE);
}

cocos2d::Vec2 AbyssMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    Vec2 candidate = next;
    float minX = s * 0.5f;
    float minY = s * 0.5f;
    float maxX = _cols * s - s * 0.5f;
    float maxY = _rows * s - s * 0.5f;
    candidate.x = std::max(minX, std::min(maxX, candidate.x));
    candidate.y = std::max(minY, std::min(maxY, candidate.y));
    return candidate;
}

bool AbyssMapController::isNearDoor(const Vec2& playerWorldPos) const {
    // 用楼梯位置代替“门”提示
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return playerWorldPos.distance(_stairsPos) <= s * 0.8f;
}

bool AbyssMapController::inBounds(int c, int r) const {
    return c >= 0 && r >= 0 && c < _cols && r < _rows;
}

std::pair<int,int> AbyssMapController::targetTile(const Vec2& playerPos, const Vec2& lastDir) const {
    int pc, pr; worldToTileIndex(playerPos, pc, pr);
    int dc = (lastDir.x > 0.1f) ? 1 : ((lastDir.x < -0.1f) ? -1 : 0);
    int dr = (lastDir.y > 0.1f) ? 1 : ((lastDir.y < -0.1f) ? -1 : 0);
    int tc = pc + dc; int tr = pr + dr;
    if (dc == 0 && dr == 0) { tr = pr - 1; }
    tc = std::max(0, std::min(_cols - 1, tc));
    tr = std::max(0, std::min(_rows - 1, tr));
    return { tc, tr };
}

void AbyssMapController::updateCursor(const Vec2& playerPos, const Vec2& lastDir) {
    if (!_mapDraw) return;
    // 简化：不绘制独立光标，地形已足够提示
}

Game::TileType AbyssMapController::getTile(int c, int r) const {
    return _tiles[r * _cols + c];
}

void AbyssMapController::setTile(int c, int r, Game::TileType t) {
    _tiles[r * _cols + c] = t;
}

Vec2 AbyssMapController::tileToWorld(int c, int r) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return Vec2(c * s + s * 0.5f, r * s + s * 0.5f);
}

void AbyssMapController::worldToTileIndex(const Vec2& p, int& c, int& r) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    c = static_cast<int>(p.x / s);
    r = static_cast<int>(p.y / s);
}

void AbyssMapController::refreshMapVisuals() {
    if (!_mapDraw) {
        _mapDraw = DrawNode::create();
        if (_worldNode) _worldNode->addChild(_mapDraw, -1);
    }
    _mapDraw->clear();
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    Color4F base;
    switch (currentTheme()) {
        case Theme::Rock: base = Color4F(0.40f,0.35f,0.30f,1.0f); break;
        case Theme::Ice:  base = Color4F(0.30f,0.45f,0.65f,1.0f); break;
        case Theme::Lava: base = Color4F(0.55f,0.30f,0.20f,1.0f); break;
    }
    for (int r = 0; r < _rows; ++r) {
        for (int c = 0; c < _cols; ++c) {
            auto center = tileToWorld(c, r);
            Vec2 a(center.x - s/2, center.y - s/2);
            Vec2 b(center.x + s/2, center.y - s/2);
            Vec2 c2(center.x + s/2, center.y + s/2);
            Vec2 d(center.x - s/2, center.y + s/2);
            Vec2 rect[4] = { a,b,c2,d };
            _mapDraw->drawSolidPoly(rect, 4, base);
            _mapDraw->drawLine(a,b, Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(b,c2,Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(c2,d, Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(d,a, Color4F(0,0,0,0.25f));
        }
    }
    // 楼梯标记
    _mapDraw->drawSolidCircle(_stairsPos, s*0.4f, 0.0f, 16, Color4F(0.95f,0.85f,0.15f,1.0f));
}

void AbyssMapController::addActorToMap(cocos2d::Node* node, int zOrder) {
    if (_worldNode) _worldNode->addChild(node, zOrder);
}

void AbyssMapController::generateFloor(int floorIndex) {
    _floor = std::max(1, std::min(120, floorIndex));
    _tiles.assign(_cols * _rows, Game::TileType::Soil);
    // 设定楼梯位置（随机靠近右下角）
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    _stairsPos = Vec2(_cols * s * 0.8f, _rows * s * 0.2f);
    // 刷新可视
    refreshMapVisuals();
}

void AbyssMapController::descend(int by) {
    setFloor(_floor + by);
}

void AbyssMapController::setFloor(int floorIndex) {
    generateFloor(floorIndex);
    unlockElevatorIfNeeded();
}

AbyssMapController::Theme AbyssMapController::currentTheme() const {
    if (_floor <= 40) return Theme::Rock;
    if (_floor <= 80) return Theme::Ice;
    return Theme::Lava;
}

void AbyssMapController::unlockElevatorIfNeeded() {
    if (_floor % 5 == 0) _elevatorFloors.insert(_floor);
}

bool AbyssMapController::isNearStairs(const Vec2& p) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return p.distance(_stairsPos) <= s * 0.8f;
}

std::vector<int> AbyssMapController::getActivatedElevatorFloors() const {
    std::vector<int> v;
    v.reserve(_elevatorFloors.size());
    for (int f : _elevatorFloors) v.push_back(f);
    std::sort(v.begin(), v.end());
    return v;
}

} // namespace Controllers