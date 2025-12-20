#pragma once

#include "cocos2d.h"
#include <vector>
#include <unordered_map>
#include "Game/Mineral.h"
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"
#include "Game/GameConfig.h"

namespace Controllers {

class MineMapController;

// 矿物系统（环境障碍唯一来源，矿洞专用）：
// - 职责：管理矿洞内矿石/石块的生成、运行时状态容器与可视化节点同步。
// - 协作对象：MineMapController 提供楼层信息与坐标转换；上层逻辑通过绑定运行时容器驱动刷新。
class MineralSystem : public EnvironmentObstacleSystemBase {
public:
    // 构造：绑定矿洞地图控制器，用于楼层与坐标转换。
    MineralSystem(MineMapController* map)
    : _map(map) {}

    // 绑定运行时矿物列表容器（系统不拥有容器，仅维护可视化与交互）。
    void bindRuntimeStorage(std::vector<Game::MineralData>* minerals) { _runtime = minerals; }

    // 清理所有矿物可视化节点并清空运行时索引。
    void clearVisuals();
    // 根据运行时矿物列表重建可视化节点与索引（会先 clearVisuals）。
    void syncVisuals();

    // 为指定楼层从候选点生成矿物节点数据（不创建可视化）。
    void generateNodesForFloor(std::vector<Game::MineralData>& outNodes,
                               const std::vector<cocos2d::Vec2>& candidates,
                               const std::vector<cocos2d::Vec2>& stairWorldPos) const;

    // 命中并尝试破坏离 worldPos 最近的矿物节点；成功命中返回 true。
    bool hitNearestNode(std::vector<Game::MineralData>& nodes,
                        const cocos2d::Vec2& worldPos,
                        int power) const;

    // 绑定可视化挂载点：矿物节点会挂到 root 下。
    void attachTo(cocos2d::Node* root) override;

    // 从瓦片坐标生成一个默认矿物障碍（占位实现，矿洞主要通过节点数据驱动）。
    bool spawnFromTile(int c, int r, const cocos2d::Vec2& tileCenter,
                       Game::MapBase* map, int tileSize) override;

    // 随机生成矿物障碍（占位实现，矿洞主要通过节点数据驱动）。
    void spawnRandom(int count, int cols, int rows,
                     const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
                     Game::MapBase* map, int tileSize,
                     const std::function<bool(int,int)>& isSafe) override;

    // 点碰撞检测：用于玩家/实体与矿物脚底的碰撞判定。
    bool collides(const cocos2d::Vec2& point, float radius, int tileSize) const override;

    // 对指定瓦片上的矿物造成伤害（占位实现；矿洞交互通常走 hitNearestNode）。
    bool damageAt(int c, int r, int amount,
                  const std::function<void(int,int,int)>& spawnDrop,
                  const std::function<void(int,int, Game::TileType)>& setTile) override;

    // 是否为空：用于地图初始化时判断是否需要从存档/生成逻辑恢复。
    bool isEmpty() const override;

private:
    MineMapController* _map = nullptr;
    cocos2d::Node* _root = nullptr;
    std::vector<Game::MineralData>* _runtime = nullptr;
    std::unordered_map<long long, Game::Mineral*> _obstacles;
};

}
