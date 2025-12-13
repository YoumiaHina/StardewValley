#pragma once

#include "cocos2d.h"
#include "Game/GameConfig.h"
#include <vector>

namespace Game {

class MapBase : public cocos2d::Node {
public:
    bool initWithFile(const std::string& tmxFile);

    cocos2d::TMXTiledMap* getTMX() const { return _tmx; }

    cocos2d::Size getMapSize() const;
    cocos2d::Size getTileSize() const;
    const cocos2d::Size& getContentSize() const override;

    cocos2d::Vec2 tileToWorld(int c, int r) const;
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const;

    static bool collidesAt(const cocos2d::Vec2& p, float radius,
                           const std::vector<cocos2d::Rect>& rects,
                           const std::vector<std::vector<cocos2d::Vec2>>& polys);

    static cocos2d::Vec2 centerFromRectsPoints(const std::vector<cocos2d::Rect>& rects,
                                               const std::vector<cocos2d::Vec2>& points);

    static bool nearRectOrPoints(const cocos2d::Vec2& p,
                                 const std::vector<cocos2d::Rect>& rects,
                                 const std::vector<cocos2d::Vec2>& points,
                                 float radiusSquared);

    static void parseWalls(cocos2d::TMXTiledMap* tmx,
                           std::vector<cocos2d::Rect>& outRects,
                           std::vector<std::vector<cocos2d::Vec2>>& outPolys,
                           cocos2d::DrawNode* debugTarget);

protected:
    cocos2d::TMXTiledMap* _tmx = nullptr;
};

} // namespace Game
