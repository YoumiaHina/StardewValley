#pragma once

#include "cocos2d.h"
#include "GameConfig.h"
#include <vector>

namespace Game {

// MineMap: TMX wrapper for mine entrance (floor 0)
class MineMap : public cocos2d::Node {
public:
    static MineMap* create(const std::string& tmxFile);
    bool initWithFile(const std::string& tmxFile);

    // Coordinate conversions
    cocos2d::Vec2 tileToWorld(int c, int r) const;
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const;

    // Collision
    bool collides(const cocos2d::Vec2& p, float radius) const;

    // Stairs / portal
    bool nearStairs(const cocos2d::Vec2& p, float radius) const;
    cocos2d::Vec2 stairsCenter() const;
    // Appear (spawn) point
    cocos2d::Vec2 appearCenter() const;
    // Door to Farm
    bool nearDoorToFarm(const cocos2d::Vec2& p) const;
    cocos2d::Vec2 doorToFarmCenter() const;

    // Getters
    cocos2d::Size getMapSize() const; // In tiles
    cocos2d::Size getTileSize() const; // In pixels
    const cocos2d::Size& getContentSize() const override; // Total pixel size
    cocos2d::TMXTiledMap* getTMX() const { return _tmx; }

private:
    cocos2d::TMXTiledMap* _tmx = nullptr;
    // Collision data
    std::vector<cocos2d::Rect> _collisionRects;
    std::vector<std::vector<cocos2d::Vec2>> _collisionPolygons;
    cocos2d::DrawNode* _debugNode = nullptr;
    // Stairs objects (rects or points)
    std::vector<cocos2d::Rect> _stairRects;
    std::vector<cocos2d::Vec2> _stairPoints;
    // Appear objects
    std::vector<cocos2d::Rect> _appearRects;
    std::vector<cocos2d::Vec2> _appearPoints;
    // DoorToFarm objects
    std::vector<cocos2d::Rect> _doorToFarmRects;
    std::vector<cocos2d::Vec2> _doorToFarmPoints;

    void parseCollision();
    void parseStairs();
    void parseAppear();
    void parseDoorToFarm();
};

} // namespace Game