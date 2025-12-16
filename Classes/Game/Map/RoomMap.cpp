// 室内房间 TMX 解析实现：
// - 加载室内 TMX 并解析床铺/房门/碰撞/回农场门等对象
// - 提供 collides/nearDoorToFarm/doorToFarmCenter 等几何查询
// - 仅承担几何与调试绘制，具体交互由上层控制器完成
#include "Game/Map/RoomMap.h"

using namespace cocos2d;

namespace Game {

RoomMap* RoomMap::create(const std::string& tmxFile) {
    RoomMap* ret = new (std::nothrow) RoomMap();
    if (ret && ret->initWithFile(tmxFile)) { ret->autorelease(); return ret; }
    CC_SAFE_DELETE(ret); return nullptr;
}

bool RoomMap::initWithFile(const std::string& tmxFile) {
    if (!MapBase::initWithFile(tmxFile)) return false;
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
    if (_collisionDebug) { _collisionDebug->removeFromParent(); _collisionDebug = nullptr; }
    _collisionDebug = DrawNode::create();
    _tmx->addChild(_collisionDebug, 999);
    MapBase::parseWalls(_tmx, _collisionRects, _collisionPolys, _collisionDebug);
}

bool RoomMap::collides(const Vec2& p, float radius) const {
    return MapBase::collidesAt(p, radius, _collisionRects, _collisionPolys);
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
    float r2 = (16.0f * 0.6f); r2 *= r2;
    return MapBase::nearRectOrPoints(p, _doorToFarmRects, _doorToFarmPoints, r2);
}

Vec2 RoomMap::doorToFarmCenter() const {
    return MapBase::centerFromRectsPoints(_doorToFarmRects, _doorToFarmPoints);
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
