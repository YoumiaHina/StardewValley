/**
 * MineMapController: 矿洞地图控制器（120层，三阶段主题）。
 * 管理楼层、主题、地形绘制、楼梯/电梯、障碍与可视。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include <unordered_set>
#include "Controllers/Map/IMapController.h"
#include "Game/GameConfig.h"
#include "Game/Map/MineMap.h"
#include "Controllers/Interact/TileSelector.h"
#include "Controllers/Environment/MineralSystem.h"
#include "Controllers/Environment/StairSystem.h"
#include "Controllers/Systems/DropSystem.h"

namespace Controllers {

/**
 * MineMapController：矿洞地图控制器。
 * - 职责：管理矿洞楼层与主题、加载入口/楼层 TMX、提供瓦片/坐标换算、碰撞与位置夹紧、楼梯/电梯/出口等交互区域判定，并转发掉落与环境障碍系统刷新。
 * - 职责边界：不在此处实现采矿/掉落的业务规则细节；矿石/楼梯等环境实体状态由对应 System 作为唯一来源维护。
 * - 协作对象：通过 IMapController 接口与 PlayerController/ToolSystem/DropSystem，以及 MineralSystem/StairSystem 等模块协作。
 */
class MineMapController : public Controllers::IMapController {
public:
    enum class Theme { Rock, Ice, Lava };

    // 构造：绑定世界节点并初始化矿石/楼梯系统。
    MineMapController(cocos2d::Node* worldNode);

    // IMapController overrides
    // 将玩家在地图局部坐标转换为世界节点坐标系位置。
    cocos2d::Vec2 getPlayerPosition(const cocos2d::Vec2& playerMapLocalPos) const override;
    // 获取当前地图内容尺寸（像素）。
    cocos2d::Size getContentSize() const override;
    // 获取地图原点偏移（用于相机计算）。
    cocos2d::Vec2 getOrigin() const override { return cocos2d::Vec2(0, 0); }
    // 将玩家下一帧位置夹紧到地图边界/碰撞规则允许范围内。
    cocos2d::Vec2 clampPosition(const cocos2d::Vec2& current,
                                 const cocos2d::Vec2& next,
                                 float radius) const override;
    // 检测指定位置是否与地图/障碍发生碰撞。
    bool collides(const cocos2d::Vec2& pos, float radius) const override;
    // 门提示替换为“楼梯/电梯”，这里近似为楼梯区域。
    bool isNearDoor(const cocos2d::Vec2& playerWorldPos) const override;
    // 是否靠近箱子（矿洞默认无箱子交互）。
    bool isNearChest(const cocos2d::Vec2& playerWorldPos) const override { return false; }
    // 获取瓦片尺寸（像素）。
    float tileSize() const override { return static_cast<float>(GameConfig::TILE_SIZE); }
    // 检查瓦片索引是否在边界内。
    bool inBounds(int c, int r) const override;
    // 计算玩家面向/点击目标瓦片索引。
    std::pair<int,int> targetTile(const cocos2d::Vec2& playerPos,
                                  const cocos2d::Vec2& lastDir) const override;
    // 更新交互光标（扇形选择）。
    void updateCursor(const cocos2d::Vec2& playerPos,
                      const cocos2d::Vec2& lastDir) override;
    // 获取瓦片类型。
    Game::TileType getTile(int c, int r) const override;
    // 设置瓦片类型（用于生成/破坏等地形变化）。
    void setTile(int c, int r, Game::TileType t) override;
    // 瓦片索引转世界坐标（瓦片中心点）。
    cocos2d::Vec2 tileToWorld(int c, int r) const override;
    // 世界坐标转瓦片索引。
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const override;

    // 获取指定障碍系统实例（矿石/岩石等）。
    EnvironmentObstacleSystemBase* obstacleSystem(ObstacleKind kind) override;
    // 获取指定障碍系统实例（只读）。
    const EnvironmentObstacleSystemBase* obstacleSystem(ObstacleKind kind) const override;
    // 刷新地图可视（地形、楼梯等）。
    void refreshMapVisuals() override;
    // 刷新作物可视（矿洞无作物，空实现）。
    void refreshCropsVisuals() override {}
    // 刷新掉落物可视。
    void refreshDropsVisuals() override;
    // 在指定瓦片生成掉落物。
    void spawnDropAt(int c, int r, int itemType, int qty) override;
    // 收集玩家附近掉落物到背包。
    void collectDropsNear(const cocos2d::Vec2& playerWorldPos, Game::Inventory* inv) override;
    // 设置动态碰撞矩形（如采矿节点临时碰撞）。
    void setDynamicColliders(const std::vector<cocos2d::Rect>& rects) { _dynamicColliders = rects; }
    // 设置怪物碰撞矩形。
    void setMonsterColliders(const std::vector<cocos2d::Rect>& rects) { _monsterColliders = rects; }
    // 碰撞检测（忽略怪物碰撞区域）。
    bool collidesWithoutMonsters(const cocos2d::Vec2& pos, float radius) const;
    // 获取箱子列表（矿洞无箱子，返回空容器引用）。
    const std::vector<Game::Chest>& chests() const override { return _emptyChests; }
    // 获取箱子列表（矿洞无箱子，返回空容器引用）。
    std::vector<Game::Chest>& chests() override { return _emptyChests; }
    // 将角色节点挂到正确父节点与层级。
    void addActorToMap(cocos2d::Node* node, int zOrder) override;

    // Mine specific
    // 生成并切换到指定楼层（1~120）。
    void generateFloor(int floorIndex);
    // 向下移动楼层（默认 -1 层）。
    void descend(int by = 1);
    // 设置当前楼层（不触发额外逻辑时使用）。
    void setFloor(int floorIndex);
    // 获取当前楼层号。
    int currentFloor() const { return _floor; }
    // 获取当前主题（岩石/冰/岩浆）。
    Theme currentTheme() const;
    // 若达到电梯解锁条件则记录解锁楼层。
    void unlockElevatorIfNeeded();
    // 是否靠近楼梯区域（用于交互提示与触发）。
    bool isNearStairs(const cocos2d::Vec2& p) const;
    // 是否靠近返回农场入口（矿洞入口层）。
    bool isNearFarmDoor(const cocos2d::Vec2& p) const;
    // 是否靠近返回入口 Back0 点。
    bool isNearBack0(const cocos2d::Vec2& p) const;
    // 是否靠近电梯楼梯区域（入口层）。
    bool isNearElestair(const cocos2d::Vec2& p) const;
    // 设置额外楼梯位置列表（用于多楼梯地图）。
    void setExtraStairs(const std::vector<cocos2d::Vec2>& stairs) { _extraStairs = stairs; }
    // 获取已激活电梯楼层列表。
    std::vector<int> getActivatedElevatorFloors() const;
    // Entrance (floor 0)
    // 加载入口层（floor 0）。
    void loadEntrance();
    // 获取入口层出生点。
    cocos2d::Vec2 entranceSpawnPos() const;
    // 返回入口时的出生点（BackAppear 优先）。
    cocos2d::Vec2 entranceBackSpawnPos() const;
    // Floor TMX loading
    // 加载指定楼层 TMX（floor 1~120）。
    void loadFloorTMX(int floorIndex);
    // 当前是否已加载楼层 TMX。
    bool isFloorTMXLoaded() const { return _floorMap != nullptr; }
    // 获取当前楼层出生点。
    cocos2d::Vec2 floorSpawnPos() const;
    // Floor TMX object groups accessors
    // 获取怪物出生点列表（从 TMX 对象层解析）。
    const std::vector<cocos2d::Vec2>& monsterSpawnPoints() const;
    // 获取矿区矩形区域列表（从 TMX 对象层解析）。
    const std::vector<cocos2d::Rect>& rockAreaRects() const;
    // 获取矿区多边形区域列表（从 TMX 对象层解析）。
    const std::vector<std::vector<cocos2d::Vec2>>& rockAreaPolys() const;

private:
    cocos2d::Node* _worldNode = nullptr;
    cocos2d::DrawNode* _mapDraw = nullptr;
    cocos2d::DrawNode* _cursor = nullptr;
    int _cols = 80;
    int _rows = 60;
    std::vector<Game::TileType> _tiles; // 简化：使用 TileType 渲染主题色块
    int _floor = 1;
    cocos2d::Vec2 _stairsPos;
    std::vector<cocos2d::Vec2> _extraStairs;
    std::unordered_set<int> _elevatorFloors; // 已激活楼层（5的倍数）
    std::vector<Game::Chest> _emptyChests; // 深渊内无箱子，返回空引用
    Controllers::DropSystem _dropSystem;
    Game::MineMap* _entrance = nullptr;
    Game::MineMap* _floorMap = nullptr;
    cocos2d::Node* _mapNode = nullptr;
    std::vector<Game::MineralData> _minerals;
    Controllers::MineralSystem _mineralSystem;
    Controllers::StairSystem _stairSystem;
    std::vector<cocos2d::Rect> _dynamicColliders; // 采矿节点临时碰撞
    std::vector<cocos2d::Rect> _monsterColliders;
    cocos2d::Vec2 _lastClickWorldPos = cocos2d::Vec2::ZERO;
    bool _hasLastClick = false;

public:
    // 记录最近一次点击的世界坐标（用于三格选择）。
    void setLastClickWorldPos(const cocos2d::Vec2& p) override;
    // 清除最近一次点击记录。
    void clearLastClickWorldPos() override { _hasLastClick = false; }
};

}
// namespace Controllers
