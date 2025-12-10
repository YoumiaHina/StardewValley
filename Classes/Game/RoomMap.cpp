#include "Game/RoomMap.h"

using namespace cocos2d;

namespace Game {

RoomMap* RoomMap::create(const std::string& tmxFile) {
    RoomMap* ret = new (std::nothrow) RoomMap();
    if (ret && ret->initWithFile(tmxFile)) { ret->autorelease(); return ret; }
    CC_SAFE_DELETE(ret); return nullptr;
}

bool RoomMap::initWithFile(const std::string& tmxFile) {
    if (!Node::init()) return false;
    _tmx = TMXTiledMap::create(tmxFile);
    if (!_tmx) return false;
    this->addChild(_tmx);
    parseObjects();
    parseCollision();
    parseDoorToFarm();
    return true;
}

void RoomMap::parseObjects() {
    _bedRects.clear();
    _hasDoor = false;

    if (!_tmx) return;
    if (auto bed = _tmx->getObjectGroup("Bed")) {
        auto objs = bed->getObjects();
        for (auto &v : objs) {
            auto m = v.asValueMap();
            float x = m.at("x").asFloat();
            float y = m.at("y").asFloat();
            float w = m.count("width") ? m.at("width").asFloat() : 0.0f;
            float h = m.count("height") ? m.at("height").asFloat() : 0.0f;
            _bedRects.emplace_back(x, y, w, h);
        }
    }
    if (auto door = _tmx->getObjectGroup("Door")) {
        auto objs = door->getObjects();
        for (auto &v : objs) {
            auto m = v.asValueMap();
            float x = m.at("x").asFloat();
            float y = m.at("y").asFloat();
            float w = m.count("width") ? m.at("width").asFloat() : 0.0f;
            float h = m.count("height") ? m.at("height").asFloat() : 0.0f;
            _doorRect = Rect(x, y, w, h);
            _hasDoor = true;
            break;
        }
    }
}

void RoomMap::parseCollision() {
    _collisionRects.clear();
    _collisionPolys.clear();
    if (!_tmx) return;
    if (_collisionDebug) { _collisionDebug->removeFromParent(); _collisionDebug = nullptr; }
    _collisionDebug = DrawNode::create();
    _tmx->addChild(_collisionDebug, 999);
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
                _collisionPolys.push_back(pts);
                // 红色线框（不填充）
                _collisionDebug->drawPoly(pts.data(), (unsigned int)pts.size(), true, Color4F(1, 0, 0, 0.9f));
            }
        } else if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
            float w = dict.at("width").asFloat();
            float h = dict.at("height").asFloat();
            Rect r(x, y, w, h);
            _collisionRects.push_back(r);
            // 红色线框（不填充）
            _collisionDebug->drawRect(r.origin, r.origin + r.size, Color4F(1, 0, 0, 0.9f));
        }
    }
}

bool RoomMap::collides(const Vec2& p, float radius) const {
    for (const auto& r : _collisionRects) {
        float cx = std::max(r.getMinX(), std::min(p.x, r.getMaxX()));
        float cy = std::max(r.getMinY(), std::min(p.y, r.getMaxY()));
        float dx = p.x - cx; float dy = p.y - cy;
        if (dx*dx + dy*dy <= radius*radius) return true;
    }
    for (const auto& poly : _collisionPolys) {
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
        j = poly.size() - 1; float r2 = radius*radius;
        for (size_t i = 0; i < poly.size(); ++i) {
            Vec2 p1 = poly[j]; Vec2 p2 = poly[i]; Vec2 d = p2 - p1;
            if (d.lengthSquared() > 0) {
                float t = (p - p1).dot(d) / d.lengthSquared(); t = std::max(0.0f, std::min(1.0f, t));
                Vec2 close = p1 + d * t; if (p.distanceSquared(close) <= r2) return true;
            }
            j = i;
        }
    }
    return false;
}

void RoomMap::parseDoorToFarm() {
    _doorToFarmRects.clear();
    _doorToFarmPoints.clear();
    if (!_tmx) return;
    if (_doorDebug) { _doorDebug->removeFromParent(); _doorDebug = nullptr; }
    _doorDebug = DrawNode::create(); _tmx->addChild(_doorDebug, 998);
    auto group = _tmx->getObjectGroup("DoorToFarm");
    if (!group) group = _tmx->getObjectGroup("doorToFarm");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &v : objects) {
        auto m = v.asValueMap();
        float x = m.at("x").asFloat(); float y = m.at("y").asFloat();
        float w = m.count("width") ? m.at("width").asFloat() : 0.0f;
        float h = m.count("height") ? m.at("height").asFloat() : 0.0f;
        if (w > 0 && h > 0) {
            Rect r(x, y, w, h); _doorToFarmRects.emplace_back(r);
            _doorDebug->drawRect(r.origin, r.origin + r.size, Color4F(0, 1, 0, 0.5f));
            _doorDebug->drawSolidRect(r.origin, r.origin + r.size, Color4F(0, 1, 0, 0.2f));
        } else {
            Vec2 pt(x, y); _doorToFarmPoints.emplace_back(pt);
            _doorDebug->drawDot(pt, 6.0f, Color4F(0, 1, 0, 0.7f));
        }
    }
}

bool RoomMap::nearDoorToFarm(const Vec2& p) const {
    for (const auto& r : _doorToFarmRects) { if (r.containsPoint(p)) return true; }
    float r2 = (16.0f * 0.6f); r2 *= r2; // tile-based fallback
    for (const auto& pt : _doorToFarmPoints) { if (p.distanceSquared(pt) <= r2) return true; }
    return false;
}

Vec2 RoomMap::doorToFarmCenter() const {
    if (!_doorToFarmRects.empty()) { const auto& r = _doorToFarmRects.front(); return Vec2(r.getMidX(), r.getMidY()); }
    if (!_doorToFarmPoints.empty()) return _doorToFarmPoints.front();
    return Vec2::ZERO;
}

Size RoomMap::getMapSize() const {
    return _tmx ? _tmx->getMapSize() : Size::ZERO;
}

Size RoomMap::getTileSize() const {
    return _tmx ? _tmx->getTileSize() : Size::ZERO;
}

const Size& RoomMap::getContentSize() const {
    if (_tmx) return _tmx->getContentSize();
    return Size::ZERO;
}

} // namespace Game

