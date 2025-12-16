// 海滩 TMX 解析实现：
// - 加载海滩 TMX 并解析墙体/水域/返回农场入口
// - 提供 collides/nearWater/nearDoorToFarm 等几何查询
// - 调试模式下以 DrawNode 可视化对象组区域
#include "Game/Map/BeachMap.h"
#include "Game/GameConfig.h"

using namespace cocos2d;

namespace Game {

BeachMap* BeachMap::create(const std::string& tmxFile) {
    auto p = new BeachMap();
    if (p && p->initWithFile(tmxFile)) {
        p->autorelease();
        return p;
    }
    CC_SAFE_DELETE(p);
    return nullptr;
}

bool BeachMap::initWithFile(const std::string& tmxFile) {
    if (!MapBase::initWithFile(tmxFile)) return false;
    parseWalls();
    parseWater();
    parseDoorToFarm();
    return true;
}

bool BeachMap::collides(const Vec2& p, float radius) const {
    if (MapBase::collidesAt(p, radius, _wallRects, _wallPolys)) return true;
    return false;
}

bool BeachMap::nearWater(const Vec2& p, float radius) const {
    float r2 = radius * radius;
    if (MapBase::nearRectOrPoints(p, _waterRects, {}, r2)) return true;
    for (const auto& poly : _waterPolys) {
        if (poly.size() >= 3) {
            size_t j = poly.size() - 1;
            for (size_t i = 0; i < poly.size(); ++i) {
                Vec2 p1 = poly[j]; Vec2 p2 = poly[i]; Vec2 d = p2 - p1;
                if (d.lengthSquared() > 0) {
                    float t = (p - p1).dot(d) / d.lengthSquared();
                    t = std::max(0.0f, std::min(1.0f, t));
                    Vec2 close = p1 + d * t;
                    if (p.distanceSquared(close) <= r2) return true;
                }
                j = i;
            }
        }
    }
    return false;
}

bool BeachMap::nearDoorToFarm(const Vec2& p) const {
    float r2 = (GameConfig::TILE_SIZE * 0.6f); r2 *= r2;
    return MapBase::nearRectOrPoints(p, _doorToFarmRects, _doorToFarmPoints, r2);
}

Vec2 BeachMap::doorToFarmCenter() const {
    return MapBase::centerFromRectsPoints(_doorToFarmRects, _doorToFarmPoints);
}

void BeachMap::parseWalls() {
    if (_wallDebug) { _wallDebug->removeFromParent(); _wallDebug = nullptr; }
    _wallDebug = DrawNode::create();
    _tmx->addChild(_wallDebug, 999);
    MapBase::parseWalls(_tmx, _wallRects, _wallPolys, _wallDebug);
}

void BeachMap::parseWater() {
    _waterRects.clear();
    _waterPolys.clear();
    if (!_tmx) return;
    if (_waterDebug) { _waterDebug->removeFromParent(); _waterDebug = nullptr; }
    _waterDebug = DrawNode::create();
    _tmx->addChild(_waterDebug, 999);
    auto group = _tmx->getObjectGroup("Water");
    if (!group) group = _tmx->getObjectGroup("water");
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
                float finalX = x + px;
                float finalY = y - py;
                pts.emplace_back(finalX, finalY);
            }
            if (!pts.empty()) {
                _waterPolys.push_back(pts);
                _waterDebug->drawPoly(pts.data(), (unsigned int)pts.size(), true, Color4F(0, 0, 1, 0.5f));
                _waterDebug->drawSolidPoly(pts.data(), (unsigned int)pts.size(), Color4F(0, 0, 1, 0.2f));
            }
        } else if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
            float w = dict.at("width").asFloat(); float h = dict.at("height").asFloat();
            Rect r(x, y, w, h);
            _waterRects.push_back(r);
            _waterDebug->drawRect(r.origin, r.origin + r.size, Color4F(0, 0, 1, 0.5f));
            _waterDebug->drawSolidRect(r.origin, r.origin + r.size, Color4F(0, 0, 1, 0.2f));
        }
    }
}

void BeachMap::parseDoorToFarm() {
    _doorToFarmRects.clear();
    _doorToFarmPoints.clear();
    if (!_tmx) return;
    if (_doorDebug) { _doorDebug->removeFromParent(); _doorDebug = nullptr; }
    _doorDebug = DrawNode::create();
    _tmx->addChild(_doorDebug, 999);
    auto group = _tmx->getObjectGroup("DoorToFarm");
    if (!group) group = _tmx->getObjectGroup("doorToFarm");
    if (!group) return;
    auto objects = group->getObjects();
    for (auto &val : objects) {
        auto dict = val.asValueMap();
        float x = dict.at("x").asFloat();
        float y = dict.at("y").asFloat();
        if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
            float w = dict.at("width").asFloat(); float h = dict.at("height").asFloat();
            Rect r(x, y, w, h);
            _doorToFarmRects.push_back(r);
            _doorDebug->drawRect(r.origin, r.origin + r.size, Color4F(1, 1, 0, 0.8f));
        } else {
            _doorToFarmPoints.emplace_back(x, y);
            _doorDebug->drawDot(Vec2(x, y), 3.0f, Color4F(1, 1, 0, 0.8f));
        }
    }
}

} // namespace Game
