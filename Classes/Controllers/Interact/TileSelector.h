#pragma once

#include "cocos2d.h"
#include <functional>
#include <utility>
#include <vector>

namespace Controllers {

class TileSelector {
public:
    static std::pair<int,int> selectForwardTile(
        const cocos2d::Vec2& playerPos,
        const cocos2d::Vec2& lastDir,
        const std::function<void(const cocos2d::Vec2&, int&, int&)>& worldToTileIndex,
        const std::function<bool(int,int)>& inBounds,
        float tileSize,
        bool hasLastClick,
        const cocos2d::Vec2& lastClickWorldPos,
        const std::function<cocos2d::Vec2(int,int)>& tileToWorld);

    static void drawFanCursor(
        cocos2d::DrawNode* cursor,
        const cocos2d::Vec2& playerPos,
        const cocos2d::Vec2& lastDir,
        const std::function<void(const cocos2d::Vec2&, int&, int&)>& worldToTileIndex,
        const std::function<bool(int,int)>& inBounds,
        const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
        float tileSize);

    static void collectForwardFanTiles(
        const cocos2d::Vec2& playerPos,
        const cocos2d::Vec2& lastDir,
        const std::function<void(const cocos2d::Vec2&, int&, int&)>& worldToTileIndex,
        const std::function<bool(int,int)>& inBounds,
        float tileSize,
        bool includeSelf,
        std::vector<std::pair<int,int>>& outTiles);
};

}
