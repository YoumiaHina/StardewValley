#pragma once

#include "cocos2d.h"

namespace Game {

class RoomMap : public cocos2d::Node {
public:
    static RoomMap* create(const std::string& tmxFile);
    bool initWithFile(const std::string& tmxFile);

    cocos2d::TMXTiledMap* getTMX() const { return _tmx; }

    cocos2d::Size getMapSize() const;
    cocos2d::Size getTileSize() const;
    const cocos2d::Size& getContentSize() const override;

    const std::vector<cocos2d::Rect>& bedRects() const { return _bedRects; }
    bool getFirstDoorRect(cocos2d::Rect& out) const { if (_hasDoor) { out = _doorRect; return true; } return false; }

private:
    cocos2d::TMXTiledMap* _tmx = nullptr;
    std::vector<cocos2d::Rect> _bedRects;
    cocos2d::Rect _doorRect;
    bool _hasDoor = false;

    void parseObjects();
};

} // namespace Game

