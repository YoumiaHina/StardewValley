#pragma once

#include "cocos2d.h"
#include <unordered_map>
#include <functional>
#include "Game/Tree.h"
#include "Game/Map/MapBase.h"
#include "Game/Tile.h"
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"

namespace Controllers {

// 树木系统（环境障碍唯一来源）：
// - 职责：管理农场树的生成/查找/碰撞/受击与销毁，并维护与 WorldState.farmTrees 的一致性。
// - 协作对象：MapBase 提供地图碰撞与区域判定；掉落与瓦片修改通过回调由上层执行。
class TreeSystem : public EnvironmentObstacleSystemBase {
public:
    // 绑定可视化挂载点：树精灵节点会挂到 root 下。
    void attachTo(cocos2d::Node* root) override;

    // 从瓦片坐标生成一棵树（随机种类），并挂接可视化到 root。
    bool spawnFromTile(int c, int r, const cocos2d::Vec2& tileCenter,
                       Game::MapBase* map, int tileSize) override;

    // 在指定瓦片位置生成指定种类的树（用于从存档还原）。
    bool spawnFromTileWithKind(int c, int r, const cocos2d::Vec2& tileCenter,
                               Game::MapBase* map, int tileSize,
                               Game::TreeKind kind);

    // 随机生成指定数量的树（在线：会创建可视化），并调用 isSafe 过滤不可生成位置。
    void spawnRandom(int count, int cols, int rows,
                     const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
                     Game::MapBase* map, int tileSize,
                     const std::function<bool(int,int)>& isSafe) override;

    // 生成农场初始树（仅在 WorldState.farmTrees 为空时），并通过占位回调避免与其它障碍重叠。
    void generateInitial(int cols, int rows,
                         const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
                         Game::MapBase* map, int tileSize,
                         const std::function<bool(int,int)>& isBlockedTile,
                         const std::function<bool(int,int)>& isOccupiedTile,
                         const std::function<void(int,int)>& markOccupiedTile);

    // 夜间补充生成：当树数量低于阈值时，向 WorldState 追加生成若干棵树（不创建可视化）。
    static int regrowNightlyWorldOnly(int cols, int rows,
                                      const std::function<Game::TileType(int,int)>& getTile,
                                      const std::function<bool(int,int)>& isOccupiedTile,
                                      const std::function<void(int,int)>& markOccupiedTile);

    // 查找指定瓦片坐标的树节点（在线：返回运行时节点指针；未找到返回 nullptr）。
    Game::Tree* findTreeAt(int c, int r) const;
    
    // 点碰撞检测：用于玩家/实体与树的脚底碰撞判定。
    bool collides(const cocos2d::Vec2& point, float radius, int tileSize) const override;

    // 对指定瓦片上的树造成伤害；若摧毁则播放动画、生成掉落，并从 WorldState 清理。
    bool damageAt(int c, int r, int amount,
                  const std::function<void(int,int,int)>& spawnDrop,
                  const std::function<void(int,int, Game::TileType)>& setTile) override;

    // 每帧排序：维护季节贴图刷新与 ZOrder（按 Y 值排序以实现遮挡关系）。
    void sortTrees();
    // 是否为空：用于地图初始化时判断是否需要从存档/生成逻辑恢复。
    bool isEmpty() const override;
    // 获取当前系统内所有树的瓦片坐标与种类（用于写回 WorldState）。
    std::vector<Game::TreePos> getAllTreeTiles() const;

private:
    cocos2d::Node* _root = nullptr;
    std::unordered_map<long long, Game::Tree*> _trees;
    int _cachedSeasonIndex = -1;
};
} // namespace Controllers
