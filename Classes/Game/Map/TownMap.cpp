#include "Game/Map/TownMap.h"
#include "Game/GameConfig.h"

using namespace cocos2d;

namespace Game {

TownMap* TownMap::create(const std::string& tmxFile) {
    auto p = new TownMap();
    if (p && p->initWithFile(tmxFile)) {
        p->autorelease();
        return p;
    }
    CC_SAFE_DELETE(p);
    return nullptr;
}

bool TownMap::initWithFile(const std::string& tmxFile) {
    if (!MapBase::initWithFile(tmxFile)) return false;
    parseWalls();
    parseDoorToFarm();
    return true;
}

bool TownMap::collides(const Vec2& p, float radius) const {
    if (MapBase::collidesAt(p, radius, _wallRects, _wallPolys)) return true;
    return false;
}

bool TownMap::nearDoorToFarm(const Vec2& p) const {
    float r2 = (GameConfig::TILE_SIZE * 0.6f); r2 *= r2;
    return MapBase::nearRectOrPoints(p, _doorToFarmRects, _doorToFarmPoints, r2);
}

Vec2 TownMap::doorToFarmCenter() const {
    return MapBase::centerFromRectsPoints(_doorToFarmRects, _doorToFarmPoints);
}

void TownMap::parseWalls() {
    if (_wallDebug) { _wallDebug->removeFromParent(); _wallDebug = nullptr; }
    _wallDebug = DrawNode::create();
    _tmx->addChild(_wallDebug, 999);
    MapBase::parseWalls(_tmx, _wallRects, _wallPolys, _wallDebug);
}

void TownMap::parseDoorToFarm() {
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

