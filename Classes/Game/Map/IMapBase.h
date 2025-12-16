#pragma once

#include "cocos2d.h"

namespace Game {

class IMapBase : public cocos2d::Node {
public:
    virtual ~IMapBase() = default;

    virtual cocos2d::TMXTiledMap* getTMX() const = 0;
    virtual cocos2d::Size getMapSize() const = 0;
    virtual cocos2d::Size getTileSize() const = 0;
    virtual const cocos2d::Size& getContentSize() const override = 0;

    virtual cocos2d::Vec2 tileToWorld(int c, int r) const = 0;
    virtual void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const = 0;

    virtual bool collides(const cocos2d::Vec2& p, float radius) const = 0;
    virtual bool inBuildingArea(const cocos2d::Vec2&) const { return false; }
};

}

