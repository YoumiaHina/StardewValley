/**
 * IMapController: 抽象地图/房间控制器接口，供 PlayerController、ToolSystem 等模块通过公共方法交互。
 */
#pragma once

#include "cocos2d.h"
#include <utility>
#include <vector>
#include "Game/Tile.h"
#include "Game/Chest.h"

namespace Controllers {

class IMapController {
public:
    virtual ~IMapController() = default;

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

    // Farm 专用：农作物/掉落
    virtual void refreshMapVisuals() {}
    virtual void refreshCropsVisuals() {}
    virtual void refreshDropsVisuals() {}
    virtual void spawnDropAt(int c, int r, int itemType /*Game::ItemType*/ , int qty) {}
    virtual int  findCropIndex(int c, int r) const { return -1; }
    virtual void plantCrop(int cropType /*Game::CropType*/, int c, int r) {}
    virtual void advanceCropsDaily() {}
    virtual void harvestCropAt(int c, int r) {}

    // Chest 容器访问（Farm/Room 实现）
    virtual const std::vector<Game::Chest>& chests() const = 0;
    virtual std::vector<Game::Chest>& chests() = 0;
};

} // namespace Controllers