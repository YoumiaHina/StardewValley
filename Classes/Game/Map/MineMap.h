#pragma once

#include "cocos2d.h"
#include "Game/GameConfig.h"
#include "Game/Map/MapBase.h"
#include <vector>

namespace Game {

// MineMap: TMX wrapper for mine entrance (floor 0)
class MineMap : public MapBase {
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
    // Back0: 返回入口（零层）
    bool nearBack0(const cocos2d::Vec2& p) const;
    // Spawn helpers for mining/monster
    const std::vector<cocos2d::Rect>& rockAreaRects() const { return _rockAreaRects; }
    const std::vector<std::vector<cocos2d::Vec2>>& rockAreaPolys() const { return _rockAreaPolys; }
    const std::vector<cocos2d::Vec2>& monsterSpawnPoints() const { return _monsterPoints; }

    // Getters
    cocos2d::Size getMapSize() const; // In tiles
    cocos2d::Size getTileSize() const; // In pixels
    const cocos2d::Size& getContentSize() const override; // Total pixel size
    cocos2d::TMXTiledMap* getTMX() const { return _tmx; }

private:
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
    // Back0 objects
    std::vector<cocos2d::Rect> _back0Rects;
    // Elevator stairs (elestair) objects：仅入口层，用于唤起电梯
    std::vector<cocos2d::Rect> _elestairRects;
    std::vector<cocos2d::Vec2> _elestairPoints;
    // BackAppear objects: 返回入口时的出生点
    std::vector<cocos2d::Rect> _backAppearRects;
    std::vector<cocos2d::Vec2> _backAppearPoints;
    // Mining / Monsters
    std::vector<cocos2d::Rect> _rockAreaRects;
    std::vector<std::vector<cocos2d::Vec2>> _rockAreaPolys;
    std::vector<cocos2d::Vec2> _monsterPoints;

    void parseCollision();
    void parseStairs();
    void parseAppear();
    void parseDoorToFarm();
    void parseBack0();
    void parseElestair();
    void parseBackAppear();
    void parseRockArea();
    void parseMonsterArea();

public:
    // 返回入口出生点（BackAppear）
    cocos2d::Vec2 backAppearCenter() const;
    // elestair 交互判定（入口层）
    bool nearElestair(const cocos2d::Vec2& p) const;
};

} // namespace Game
