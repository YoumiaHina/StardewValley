// 室内房间 TMX 地图封装：
// - 基于 MapBase 提供卧室/室内场景的尺寸与碰撞查询
// - 解析床铺区域与房门矩形供睡觉/出门等交互使用
// - 仅负责 TMX 几何与对象解析，不直接包含业务逻辑
#pragma once

#include "cocos2d.h"
#include "Game/Map/MapBase.h"

namespace Game {

class RoomMap : public MapBase {
public:
    static RoomMap* create(const std::string& tmxFile);
    bool initWithFile(const std::string& tmxFile);

    cocos2d::TMXTiledMap* getTMX() const { return _tmx; }

    cocos2d::Size getMapSize() const;
    cocos2d::Size getTileSize() const;
    const cocos2d::Size& getContentSize() const override;

    const std::vector<cocos2d::Rect>& bedRects() const { return _bedRects; }
    bool getFirstDoorRect(cocos2d::Rect& out) const { if (_hasDoor) { out = _doorRect; return true; } return false; }

    // Collision
    bool collides(const cocos2d::Vec2& p, float radius) const;

    // Door to Farm
    bool nearDoorToFarm(const cocos2d::Vec2& p) const;
    cocos2d::Vec2 doorToFarmCenter() const;

private:
    std::vector<cocos2d::Rect> _bedRects;
    cocos2d::Rect _doorRect;
    bool _hasDoor = false;

    // Collision data
    std::vector<cocos2d::Rect> _collisionRects;
    std::vector<std::vector<cocos2d::Vec2>> _collisionPolys;
    cocos2d::DrawNode* _collisionDebug = nullptr;

    // DoorToFarm objects
    std::vector<cocos2d::Rect> _doorToFarmRects;
    std::vector<cocos2d::Vec2> _doorToFarmPoints;
    cocos2d::DrawNode* _doorDebug = nullptr;

    void parseObjects();
    void parseCollision();
    void parseDoorToFarm();
};

} // namespace Game
