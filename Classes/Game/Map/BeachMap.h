#pragma once

#include "cocos2d.h"
#include "Game/Map/MapBase.h"

namespace Game {

class BeachMap : public MapBase {
public:
    static BeachMap* create(const std::string& tmxFile);
    bool initWithFile(const std::string& tmxFile);

    cocos2d::TMXTiledMap* getTMX() const { return _tmx; }

    bool collides(const cocos2d::Vec2& p, float radius) const;

    bool nearDoorToFarm(const cocos2d::Vec2& p) const;
    cocos2d::Vec2 doorToFarmCenter() const;

    bool nearWater(const cocos2d::Vec2& p, float radius) const;

private:
    std::vector<cocos2d::Rect> _wallRects;
    std::vector<std::vector<cocos2d::Vec2>> _wallPolys;
    cocos2d::DrawNode* _wallDebug = nullptr;

    std::vector<cocos2d::Rect> _waterRects;
    std::vector<std::vector<cocos2d::Vec2>> _waterPolys;
    cocos2d::DrawNode* _waterDebug = nullptr;

    std::vector<cocos2d::Rect> _doorToFarmRects;
    std::vector<cocos2d::Vec2> _doorToFarmPoints;
    cocos2d::DrawNode* _doorDebug = nullptr;

    void parseWalls();
    void parseWater();
    void parseDoorToFarm();
};

} // namespace Game

