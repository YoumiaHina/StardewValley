// 小镇 TMX 地图封装：
// - 基于 MapBase 提供小镇场景的碰撞与门口查询
// - 解析回农场传送门几何数据，供场景切换判定使用
// - 仅负责 TMX 几何与图层顺序，不参与业务逻辑
#pragma once

#include "cocos2d.h"
#include "Game/Map/MapBase.h"

namespace Game {

class TownMap : public MapBase {
public:
    static TownMap* create(const std::string& tmxFile);
    bool initWithFile(const std::string& tmxFile);

    cocos2d::TMXTiledMap* getTMX() const { return _tmx; }

    bool collides(const cocos2d::Vec2& p, float radius) const;

    bool nearDoorToFarm(const cocos2d::Vec2& p) const;
    cocos2d::Vec2 doorToFarmCenter() const;
    bool nearToolUpdateArea(const cocos2d::Vec2& p) const;

    // 设置节日开关：控制 Festival 图层显示，并切换碰撞对象层来源。
    void setFestivalActive(bool active);

private:
    cocos2d::TMXLayer* _bgLayer = nullptr;
    cocos2d::TMXLayer* _houseBodyLayer = nullptr;
    cocos2d::TMXLayer* _houseTopLayer = nullptr;
    cocos2d::TMXLayer* _festivalLayer = nullptr;

    std::vector<cocos2d::Rect> _wallRects;
    std::vector<std::vector<cocos2d::Vec2>> _wallPolys;
    std::vector<cocos2d::Rect> _festivalWallRects;
    std::vector<std::vector<cocos2d::Vec2>> _festivalWallPolys;
    cocos2d::DrawNode* _wallDebug = nullptr;
    bool _festivalActive = false;

    std::vector<cocos2d::Rect> _doorToFarmRects;
    std::vector<cocos2d::Vec2> _doorToFarmPoints;
    cocos2d::DrawNode* _doorDebug = nullptr;

    std::vector<cocos2d::Rect> _toolUpdateRects;
    std::vector<cocos2d::Vec2> _toolUpdatePoints;
    cocos2d::DrawNode* _toolUpdateDebug = nullptr;

    void parseWalls();
    void parseDoorToFarm();
    void parseToolUpdateArea();
    void setupLayerOrder();
};

} // namespace Game
