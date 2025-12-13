#include "Game/MineMap.h"

using namespace cocos2d;

namespace Game {

MineMap* MineMap::create(const std::string& tmxFile) {
    MineMap* ret = new (std::nothrow) MineMap();
    if (ret && ret->initWithFile(tmxFile)) { ret->autorelease(); return ret; }
    CC_SAFE_DELETE(ret); return nullptr;
}

bool MineMap::initWithFile(const std::string& tmxFile) {
    if (!Node::init()) return false;
    _tmx = TMXTiledMap::create(tmxFile);
    if (!_tmx) return false;
    this->addChild(_tmx);
    parseCollision();
    parseStairs();
    parseAppear();
    parseDoorToFarm();
    parseBack0();
    parseElestair();
    parseBackAppear();
    parseRockArea();
    parseMonsterArea();
    return true;
}

void MineMap::parseCollision() {
    _collisionRects.clear();
    _collisionPolygons.clear();
    if (!_tmx) return;
    if (_debugNode) { _debugNode->removeFromParent(); _debugNode = nullptr; }
    _debugNode = DrawNode::create();
    _tmx->addChild(_debugNode, 999);
    auto group = _tmx->getObjectGroup("collision");
    if (!group) group = _tmx->getObjectGroup("Collision");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        if (dict.find("points") != dict.end() || dict.find("polyline") != dict.end() || dict.find("polygon") != dict.end()) {
            std::vector<Vec2> pts;
            ValueVector arr;
            if (dict.find("points") != dict.end()) arr = dict.at("points").asValueVector();
            else if (dict.find("polygon") != dict.end()) arr = dict.at("polygon").asValueVector();
            else if (dict.find("polyline") != dict.end()) arr = dict.at("polyline").asValueVector();
            for (auto &pv : arr) {
                auto pmap = pv.asValueMap();
                float px = pmap.at("x").asFloat();
                float py = pmap.at("y").asFloat();
                float finalX = x + px;
                float finalY = y - py;
                pts.emplace_back(finalX, finalY);
            }
            if (!pts.empty()) {
                _collisionPolygons.push_back(pts);
                _debugNode->drawPoly(pts.data(), (unsigned int)pts.size(), true, Color4F(1, 0, 0, 0.5f));
                _debugNode->drawSolidPoly(pts.data(), (unsigned int)pts.size(), Color4F(1, 0, 0, 0.2f));
            }
        } else if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
            float w = dict.at("width").asFloat();
            float h = dict.at("height").asFloat();
            Rect r(x, y, w, h);
            _collisionRects.push_back(r);
            _debugNode->drawRect(r.origin, r.origin + r.size, Color4F(1, 0, 0, 0.5f));
            _debugNode->drawSolidRect(r.origin, r.origin + r.size, Color4F(1, 0, 0, 0.2f));
        }
    }
}

void MineMap::parseStairs() {
    _stairRects.clear();
    _stairPoints.clear();
    if (!_tmx) return;
    auto group = _tmx->getObjectGroup("stair");
    if (!group) group = _tmx->getObjectGroup("Stair");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        float w = dict.count("width") ? dict.at("width").asFloat() : 0.0f;
        float h = dict.count("height") ? dict.at("height").asFloat() : 0.0f;
        if (w > 0 && h > 0) {
            _stairRects.emplace_back(x, y, w, h);
        } else {
            _stairPoints.emplace_back(x, y);
        }
    }
}

void MineMap::parseAppear() {
    _appearRects.clear();
    _appearPoints.clear();
    if (!_tmx) return;
    auto group = _tmx->getObjectGroup("Appear");
    if (!group) group = _tmx->getObjectGroup("appear");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        float w = dict.count("width") ? dict.at("width").asFloat() : 0.0f;
        float h = dict.count("height") ? dict.at("height").asFloat() : 0.0f;
        if (w > 0 && h > 0) {
            _appearRects.emplace_back(x, y, w, h);
        } else {
            _appearPoints.emplace_back(x, y);
        }
    }
}

bool MineMap::collides(const Vec2& p, float radius) const {
    for (const auto& r : _collisionRects) {
        float cx = std::max(r.getMinX(), std::min(p.x, r.getMaxX()));
        float cy = std::max(r.getMinY(), std::min(p.y, r.getMaxY()));
        float dx = p.x - cx;
        float dy = p.y - cy;
        if (dx*dx + dy*dy <= radius*radius) return true;
    }
    for (const auto& poly : _collisionPolygons) {
        if (poly.size() < 3) continue;
        bool inside = false; size_t j = poly.size() - 1;
        for (size_t i = 0; i < poly.size(); ++i) {
            if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
                (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x)) {
                inside = !inside;
            }
            j = i;
        }
        if (inside) return true;
        j = poly.size() - 1;
        float r2 = radius * radius;
        for (size_t i = 0; i < poly.size(); ++i) {
            Vec2 p1 = poly[j]; Vec2 p2 = poly[i]; Vec2 d = p2 - p1;
            if (d.lengthSquared() > 0) {
                float t = (p - p1).dot(d) / d.lengthSquared(); t = std::max(0.0f, std::min(1.0f, t));
                Vec2 closest = p1 + d * t; if (p.distanceSquared(closest) <= r2) return true;
            }
            j = i;
        }
    }
    return false;
}

bool MineMap::nearStairs(const Vec2& p, float radius) const {
    float r2 = radius * radius;
    for (const auto& r : _stairRects) {
        float cx = std::max(r.getMinX(), std::min(p.x, r.getMaxX()));
        float cy = std::max(r.getMinY(), std::min(p.y, r.getMaxY()));
        float dx = p.x - cx; float dy = p.y - cy; if (dx*dx + dy*dy <= r2) return true;
    }
    for (const auto& pt : _stairPoints) {
        if (p.distanceSquared(pt) <= r2) return true;
    }
    return false;
}

Vec2 MineMap::stairsCenter() const {
    if (!_stairRects.empty()) {
        const auto& r = _stairRects.front();
        return Vec2(r.getMidX(), r.getMidY());
    }
    if (!_stairPoints.empty()) return _stairPoints.front();
    return Vec2::ZERO;
}

Vec2 MineMap::appearCenter() const {
    if (!_appearRects.empty()) {
        const auto& r = _appearRects.front();
        return Vec2(r.getMidX(), r.getMidY());
    }
    if (!_appearPoints.empty()) return _appearPoints.front();
    return Vec2::ZERO;
}

void MineMap::parseDoorToFarm() {
    _doorToFarmRects.clear();
    _doorToFarmPoints.clear();
    if (!_tmx) return;
    auto group = _tmx->getObjectGroup("DoorToFarm");
    if (!group) group = _tmx->getObjectGroup("doorToFarm");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        float w = dict.count("width") ? dict.at("width").asFloat() : 0.0f;
        float h = dict.count("height") ? dict.at("height").asFloat() : 0.0f;
        if (w > 0 && h > 0) {
            _doorToFarmRects.emplace_back(x, y, w, h);
            // 调试绘制（绿色格）
            if (!_debugNode) { _debugNode = DrawNode::create(); _tmx->addChild(_debugNode, 998); }
            Rect r(x, y, w, h);
            _debugNode->drawRect(r.origin, r.origin + r.size, Color4F(0, 1, 0, 0.5f));
            _debugNode->drawSolidRect(r.origin, r.origin + r.size, Color4F(0, 1, 0, 0.2f));
        } else {
            _doorToFarmPoints.emplace_back(x, y);
            if (!_debugNode) { _debugNode = DrawNode::create(); _tmx->addChild(_debugNode, 998); }
            _debugNode->drawDot(Vec2(x, y), 6.0f, Color4F(0, 1, 0, 0.7f));
        }
    }
}

bool MineMap::nearDoorToFarm(const Vec2& p) const {
    for (const auto& r : _doorToFarmRects) {
        if (r.containsPoint(p)) return true;
    }
    // points: small radius check
    float r2 = (GameConfig::TILE_SIZE * 0.6f); r2 *= r2;
    for (const auto& pt : _doorToFarmPoints) {
        if (p.distanceSquared(pt) <= r2) return true;
    }
    return false;
}

Vec2 MineMap::doorToFarmCenter() const {
    if (!_doorToFarmRects.empty()) {
        const auto& r = _doorToFarmRects.front();
        return Vec2(r.getMidX(), r.getMidY());
    }
    if (!_doorToFarmPoints.empty()) return _doorToFarmPoints.front();
    return Vec2::ZERO;
}

void MineMap::parseBack0() {
    _back0Rects.clear();
    if (!_tmx) return;
    auto group = _tmx->getObjectGroup("Back0");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        float w = dict.count("width") ? dict.at("width").asFloat() : 0.0f;
        float h = dict.count("height") ? dict.at("height").asFloat() : 0.0f;
        if (w > 0 && h > 0) {
            _back0Rects.emplace_back(x, y, w, h);
        }
    }
}

bool MineMap::nearBack0(const Vec2& p) const {
    for (const auto& r : _back0Rects) {
        if (r.containsPoint(p)) return true;
    }
    return false;
}

void MineMap::parseElestair() {
    _elestairRects.clear();
    _elestairPoints.clear();
    if (!_tmx) return;
    auto group = _tmx->getObjectGroup("elestair");
    if (!group) group = _tmx->getObjectGroup("Elestair");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        float w = dict.count("width") ? dict.at("width").asFloat() : 0.0f;
        float h = dict.count("height") ? dict.at("height").asFloat() : 0.0f;
        if (w > 0 && h > 0) {
            _elestairRects.emplace_back(x, y, w, h);
        } else {
            _elestairPoints.emplace_back(x, y);
        }
    }
}

bool MineMap::nearElestair(const Vec2& p) const {
    for (const auto& r : _elestairRects) {
        if (r.containsPoint(p)) return true;
    }
    float r2 = (GameConfig::TILE_SIZE * 0.6f); r2 *= r2;
    for (const auto& pt : _elestairPoints) {
        if (p.distanceSquared(pt) <= r2) return true;
    }
    return false;
}

void MineMap::parseBackAppear() {
    _backAppearRects.clear();
    _backAppearPoints.clear();
    if (!_tmx) return;
    auto group = _tmx->getObjectGroup("BackAppear");
    if (!group) group = _tmx->getObjectGroup("backAppear");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        float w = dict.count("width") ? dict.at("width").asFloat() : 0.0f;
        float h = dict.count("height") ? dict.at("height").asFloat() : 0.0f;
        if (w > 0 && h > 0) {
            _backAppearRects.emplace_back(x, y, w, h);
        } else {
            _backAppearPoints.emplace_back(x, y);
        }
    }
}

cocos2d::Vec2 MineMap::backAppearCenter() const {
    if (!_backAppearRects.empty()) {
        const auto& r = _backAppearRects.front();
        return Vec2(r.getMidX(), r.getMidY());
    }
    if (!_backAppearPoints.empty()) return _backAppearPoints.front();
    return Vec2::ZERO;
}

void MineMap::parseRockArea() {
    _rockAreaRects.clear();
    _rockAreaPolys.clear();
    if (!_tmx) return;
    auto group = _tmx->getObjectGroup("RockArea");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        if (dict.find("points") != dict.end() || dict.find("polyline") != dict.end() || dict.find("polygon") != dict.end()) {
            std::vector<Vec2> pts; ValueVector arr;
            if (dict.find("points") != dict.end()) arr = dict.at("points").asValueVector();
            else if (dict.find("polygon") != dict.end()) arr = dict.at("polygon").asValueVector();
            else if (dict.find("polyline") != dict.end()) arr = dict.at("polyline").asValueVector();
            for (auto &pv : arr) {
                auto pmap = pv.asValueMap();
                float px = pmap.at("x").asFloat();
                float py = pmap.at("y").asFloat();
                pts.emplace_back(x + px, y - py);
            }
            if (!pts.empty()) _rockAreaPolys.push_back(pts);
        } else if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
            float w = dict.at("width").asFloat(); float h = dict.at("height").asFloat();
            _rockAreaRects.emplace_back(x, y, w, h);
        }
    }
}

void MineMap::parseMonsterArea() {
    _monsterPoints.clear();
    if (!_tmx) return;
    auto group = _tmx->getObjectGroup("MonsterArea");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        // point-only expected; for rects, use center
        float w = dict.count("width") ? dict.at("width").asFloat() : 0.0f;
        float h = dict.count("height") ? dict.at("height").asFloat() : 0.0f;
        if (w > 0 && h > 0) {
            _monsterPoints.emplace_back(x + w*0.5f, y + h*0.5f);
        } else {
            _monsterPoints.emplace_back(x, y);
        }
    }
}

Size MineMap::getMapSize() const {
    return _tmx ? _tmx->getMapSize() : Size::ZERO;
}

Size MineMap::getTileSize() const {
    return _tmx ? _tmx->getTileSize() : Size(GameConfig::TILE_SIZE, GameConfig::TILE_SIZE);
}

const Size& MineMap::getContentSize() const {
    if (_tmx) return _tmx->getContentSize();
    return Size::ZERO;
}

Vec2 MineMap::tileToWorld(int c, int r) const {
    float s = _tmx ? _tmx->getTileSize().width : (float)GameConfig::TILE_SIZE;
    return Vec2(c * s + s * 0.5f, r * s + s * 0.5f);
}

void MineMap::worldToTileIndex(const Vec2& p, int& c, int& r) const {
    float s = _tmx ? _tmx->getTileSize().width : (float)GameConfig::TILE_SIZE;
    c = static_cast<int>(p.x / s);
    r = static_cast<int>(p.y / s);
}

} // namespace Game