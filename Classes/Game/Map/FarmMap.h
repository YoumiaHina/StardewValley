// 农场 TMX 地图封装：
// - 基于 MapBase 提供农场场景的坐标与碰撞查询
// - 解析水域/房屋/传送门/禁树区域等对象供控制器使用
// - 仅承担几何与图层信息，不直接包含业务逻辑
#pragma once

#include "cocos2d.h"
#include "Game/GameConfig.h"
#include "Game/Map/MapBase.h"

namespace Game {

class FarmMap : public MapBase {
public:
    static FarmMap* create(const std::string& tmxFile);
    bool initWithFile(const std::string& tmxFile);

    // Coordinate conversions
    cocos2d::Vec2 tileToWorld(int c, int r) const;
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const;

    // Collision
    bool collides(const cocos2d::Vec2& p, float radius) const;

    // Getters
    cocos2d::Size getMapSize() const; // In tiles
    cocos2d::Size getTileSize() const; // In pixels
    const cocos2d::Size& getContentSize() const override; // Total pixel size

    cocos2d::TMXTiledMap* getTMX() const { return _tmx; }
    bool nearWater(const cocos2d::Vec2& p, float radius) const;
    bool nearDoorToRoom(const cocos2d::Vec2& p) const;
    bool nearDoorToMine(const cocos2d::Vec2& p) const;
    bool nearDoorToBeach(const cocos2d::Vec2& p) const;
    bool nearDoorToTown(const cocos2d::Vec2& p) const;
    cocos2d::Vec2 doorToMineCenter() const;
    cocos2d::Vec2 doorToRoomCenter() const;
    cocos2d::Vec2 doorToBeachCenter() const;
    cocos2d::Vec2 doorToTownCenter() const;
    bool inNoTreeArea(const cocos2d::Vec2& p) const;

private:
    cocos2d::TMXLayer* _bgLayer = nullptr;
    cocos2d::TMXLayer* _houseBodyLayer = nullptr;
    cocos2d::TMXLayer* _houseTopLayer = nullptr;

    // Collision data
    std::vector<cocos2d::Rect> _wallRects;
    std::vector<std::vector<cocos2d::Vec2>> _wallPolygons;
    cocos2d::DrawNode* _debugNode = nullptr;
    std::vector<cocos2d::Rect> _waterRects;
    std::vector<std::vector<cocos2d::Vec2>> _waterPolygons;
    cocos2d::DrawNode* _waterDebugNode = nullptr;
    std::vector<cocos2d::Rect> _doorToRoomRects;
    std::vector<cocos2d::Rect> _doorToMineRects;
    std::vector<cocos2d::Rect> _doorToBeachRects;
    std::vector<cocos2d::Rect> _doorToTownRects;
    cocos2d::DrawNode* _doorDebugNode = nullptr;
    std::vector<cocos2d::Rect> _noTreeRects;
    std::vector<std::vector<cocos2d::Vec2>> _noTreePolygons;

    void setupLayerOrder();
    void parseWalls();
    void parseWater();
    void parseDoorToRoom();
    void parseDoorToMine();
    void parseDoorToBeach();
    void parseDoorToTown();
    void parseNoTree();
};

} // namespace Game
