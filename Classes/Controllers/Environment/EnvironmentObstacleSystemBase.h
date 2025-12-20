#pragma once

#include "cocos2d.h"
#include <functional>
#include "Game/Map/MapBase.h"
#include "Game/Tile.h"

namespace Controllers {

// 环境障碍系统基类（接口约束）：
// - 职责：定义“环境障碍唯一来源”系统的最小协作接口（挂载、生成、碰撞、受击与清空判定）。
// - 协作对象：MapBase 用于区域/碰撞判定；上层 Controller 负责调用时机编排。
class EnvironmentObstacleSystemBase {
public:
    virtual ~EnvironmentObstacleSystemBase() = default;

    // 绑定可视化挂载点：系统创建的障碍节点应挂到 root 下。
    virtual void attachTo(cocos2d::Node* root) = 0;

    // 从瓦片坐标生成一个障碍实体（在线：创建并挂接可视化；失败返回 false）。
    virtual bool spawnFromTile(int c, int r, const cocos2d::Vec2& tileCenter,
                               Game::MapBase* map, int tileSize) = 0;

    // 随机生成指定数量的障碍（在线：创建可视化），并用 isSafe 过滤不可生成位置。
    virtual void spawnRandom(int count, int cols, int rows,
                             const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
                             Game::MapBase* map, int tileSize,
                             const std::function<bool(int,int)>& isSafe) = 0;

    // 点碰撞检测：用于玩家/实体与障碍脚底的碰撞判定。
    virtual bool collides(const cocos2d::Vec2& point, float radius, int tileSize) const = 0;

    // 对指定瓦片上的障碍造成伤害；由系统决定是否摧毁并通过回调触发掉落/瓦片修改。
    virtual bool damageAt(int c, int r, int amount,
                          const std::function<void(int,int,int)>& spawnDrop,
                          const std::function<void(int,int, Game::TileType)>& setTile) = 0;

    // 是否为空：用于地图初始化时判断是否需要从存档/生成逻辑恢复。
    virtual bool isEmpty() const = 0;
};

}
