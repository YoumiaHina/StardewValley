/**
 * IMapController: 抽象地图/房间控制器接口，供 PlayerController、ToolSystem 等模块通过公共方法交互。
 */
#pragma once

#include "cocos2d.h"
#include <utility>
#include <vector>
#include "Game/Tile.h"
#include "Game/Chest.h"
#include "Game/Inventory.h"

namespace Controllers {

class EnvironmentObstacleSystemBase;

enum class ObstacleKind {
    Mineral,
    Rock,
    Tree,
    Weed,
};

class IMapController {
public:
    virtual ~IMapController() = default;

    virtual cocos2d::Vec2 getPlayerPosition(const cocos2d::Vec2& playerMapLocalPos) const = 0;

    // 地图尺寸与原点（用于相机计算）
    virtual cocos2d::Size getContentSize() const = 0;
    virtual cocos2d::Vec2 getOrigin() const = 0;

    // 位置夹紧与碰撞（依据具体地图）
    virtual cocos2d::Vec2 clampPosition(const cocos2d::Vec2& current,
                                         const cocos2d::Vec2& next,
                                         float radius) const = 0;
    virtual bool collides(const cocos2d::Vec2& pos, float radius) const = 0;

    // 门/交互区域检测
    virtual bool isNearDoor(const cocos2d::Vec2& playerWorldPos) const = 0;
    virtual bool isNearChest(const cocos2d::Vec2& playerWorldPos) const = 0;
    // 额外门：矿洞入口（默认不支持）
    virtual bool isNearMineDoor(const cocos2d::Vec2& playerWorldPos) const { return false; }

    // Farm 专用：瓦片/光标（Room 可返回默认值）
    virtual float tileSize() const { return 0.0f; }
    virtual bool inBounds(int c, int r) const { return false; }
    virtual std::pair<int,int> targetTile(const cocos2d::Vec2& playerPos,
                                          const cocos2d::Vec2& lastDir) const { return {0,0}; }
    virtual void updateCursor(const cocos2d::Vec2& playerPos,
                              const cocos2d::Vec2& lastDir) {}

    // Farm 专用：读写瓦片类型
    virtual Game::TileType getTile(int c, int r) const { return Game::TileType::Soil; }
    virtual void setTile(int c, int r, Game::TileType t) {}
    virtual cocos2d::Vec2 tileToWorld(int c, int r) const { return cocos2d::Vec2(); }
    virtual void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const { c = 0; r = 0; }

    // 天气系统：默认不支持（仅 Farm/Beach/Town 开启）
    virtual bool supportsWeather() const { return false; }
    virtual void setAllPlantableTilesWatered() {}

    // 节日系统：默认不支持（仅 Beach/Town 需要图层/碰撞切换）
    virtual void setFestivalActive(bool) {}

    virtual EnvironmentObstacleSystemBase* obstacleSystem(ObstacleKind) { return nullptr; }
    virtual const EnvironmentObstacleSystemBase* obstacleSystem(ObstacleKind) const { return nullptr; }

    // Farm 专用：湖边判定（默认不支持）
    virtual bool isNearLake(const cocos2d::Vec2& playerWorldPos, float radius) const { return false; }
    // 矿洞入口（矿→农场）判定（默认不支持）
    virtual bool isNearFarmDoor(const cocos2d::Vec2& playerWorldPos) const { return false; }
    // 沙滩入口（农场→沙滩）判定（默认不支持）
    virtual bool isNearBeachDoor(const cocos2d::Vec2& playerWorldPos) const { return false; }
    // 城镇入口（农场→城镇）判定（默认不支持）
    virtual bool isNearTownDoor(const cocos2d::Vec2& playerWorldPos) const { return false; }
    // 农场 DoorToMine 出生点（默认 Vec2::ZERO）
    virtual cocos2d::Vec2 farmMineDoorSpawnPos() const { return cocos2d::Vec2::ZERO; }
    // 农场 DoorToRoom 出生点（默认 Vec2::ZERO）
    virtual cocos2d::Vec2 farmRoomDoorSpawnPos() const { return cocos2d::Vec2::ZERO; }
    // 农场 DoorToBeach 出生点（默认 Vec2::ZERO）
    virtual cocos2d::Vec2 farmBeachDoorSpawnPos() const { return cocos2d::Vec2::ZERO; }
    // 农场 DoorToTown 出生点（默认 Vec2::ZERO）
    virtual cocos2d::Vec2 farmTownDoorSpawnPos() const { return cocos2d::Vec2::ZERO; }

    // 鼠标交互：记录最近一次点击的世界坐标（用于三格选择）
    virtual void setLastClickWorldPos(const cocos2d::Vec2& /*p*/) {}
    virtual void clearLastClickWorldPos() {}

    // Farm 专用：渲染与掉落
    virtual void refreshMapVisuals() {}
    virtual void refreshCropsVisuals() {}
    virtual void refreshDropsVisuals() {}
    virtual void spawnDropAt(int c, int r, int itemType /*Game::ItemType*/ , int qty) {}
    virtual void collectDropsNear(const cocos2d::Vec2& playerWorldPos, Game::Inventory* inv) {}

    // Chest 容器访问（Farm/Room 实现）
    virtual const std::vector<Game::Chest>& chests() const = 0;
    virtual std::vector<Game::Chest>& chests() = 0;

    // 将角色/演员节点添加到地图正确的父节点与层级（Farm 使用 TMX，Room 使用 world）。
    virtual void addActorToMap(cocos2d::Node* node, int zOrder) = 0;

    // Actors sorting and interactive objects
    virtual void sortActorWithEnvironment(cocos2d::Node* actor) {}

    // Type check
    virtual bool isFarm() const { return false; }

    virtual void setCollisionDebugVisible(bool visible) {}
};

} // namespace Controllers
