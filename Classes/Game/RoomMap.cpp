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

