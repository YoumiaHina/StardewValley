#pragma once

#include "cocos2d.h"
#include <vector>
#include <functional>
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"
#include "Game/EnvironmentObstacle/Mineral.h"
#include "Game/EnvironmentObstacle/Stair.h"

namespace Controllers {

class MineMapController;

// 楼梯系统（环境障碍唯一来源，矿洞专用）：
// - 职责：管理矿洞楼梯点位的生成、遮挡判定与可视化刷新，并同步可用楼梯列表给地图控制器。
// - 协作对象：MineMapController 提供坐标转换与 extraStairs 写入；MineralSystem 提供遮挡来源数据。
class StairSystem : public EnvironmentObstacleSystemBase {
public:
    StairSystem(MineMapController* map)
    : _map(map) {}

    // 绑定可视化挂载点：楼梯节点与调试 DrawNode 会挂到 root 下。
    void attachTo(cocos2d::Node* root) override;

    // 重置系统：清理楼梯节点与内部列表，并清空调试绘制。
    void reset();

    // 从瓦片坐标生成楼梯（占位实现：楼梯由 generateStairs + refreshVisuals 驱动）。
    bool spawnFromTile(int c, int r, const cocos2d::Vec2& tileCenter,
                       Game::MapBase* map, int tileSize) override;

    // 随机生成楼梯（占位实现：楼梯由 generateStairs + refreshVisuals 驱动）。
    void spawnRandom(int count, int cols, int rows,
                     const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
                     Game::MapBase* map, int tileSize,
                     const std::function<bool(int,int)>& isSafe) override;

    // 点碰撞检测（占位实现：楼梯交互由地图/其它逻辑判定）。
    bool collides(const cocos2d::Vec2& point, float radius, int tileSize) const override;

    // 受击处理（占位实现：楼梯不可被工具破坏）。
    bool damageAt(int c, int r, int amount,
                  const std::function<void(int,int,int)>& spawnDrop,
                  const std::function<void(int,int, Game::TileType)>& setTile) override;

    // 是否为空：用于地图初始化时判断是否需要生成楼梯。
    bool isEmpty() const override;

    // 从候选点生成楼梯位置列表（输出为对齐到瓦片中心的世界坐标），并写入系统内部状态。
    void generateStairs(const std::vector<cocos2d::Vec2>& candidates,
                        int minCount,
                        int maxCount,
                        std::vector<cocos2d::Vec2>& outWorldPos);

    // 根据矿物节点遮挡楼梯：将被覆盖的楼梯标记为不可见，并同步可用楼梯到地图控制器。
    void syncExtraStairsToMap(const std::vector<Game::MineralData>& minerals);

    // 刷新楼梯可视化：按系统内部列表创建/更新楼梯节点并应用遮挡可见性。
    void refreshVisuals();

private:
    struct StairData {
        cocos2d::Vec2 pos;
        Game::Stair* node = nullptr;
        bool covered = false;
    };

    MineMapController* _map = nullptr;
    cocos2d::Node* _root = nullptr;
    cocos2d::DrawNode* _debugDraw = nullptr;
    std::vector<StairData> _stairs;
};

}
