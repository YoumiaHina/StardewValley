#pragma once

#include "cocos2d.h"
// Chest：游戏中“箱子”实体的数据与碰撞定义，作为箱子相关系统的基础数据结构。
// - 职责：描述箱子的容量、槽位数组与基础占用/碰撞矩形，不包含任何高层业务规则。
// - 协作：由 ChestController 等 System 维护列表并与 WorldState 同步；UI 通过 Game::Chest
//   读取/修改槽位状态；地图控制器通过碰撞矩形参与移动/挡路判定。
// - 约束：不要在此文件加入背包/掉落等流程逻辑，保持“箱子数据结构”单一职责。
#include <memory>
#include <vector>
#include "Game/GameConfig.h"
#include "Game/Inventory.h"
#include "Game/PlaceableItemBase.h"

namespace Controllers {
// 冗余前向声明：本头文件内部未直接使用，如无外部依赖可在重构时考虑移除。
class IMapController;
class UIController;
class RoomMapController;
}

namespace Game {

// Chest：单个箱子实例的数据结构。
// - slots：内部 3x12 固定容量的槽位数组（托管物品/工具）。
// - MAX_PER_AREA：用于限制同一地图区域内可放置的箱子数量。
struct Chest : public PlaceableItemBase {
    static constexpr int ROWS = 3;                 // UI 行数
    static constexpr int COLS = 12;                // UI 列数
    static constexpr int CAPACITY = ROWS * COLS;   // 槽位总数
    static constexpr int MAX_PER_AREA = 200;       // 单张地图最多箱子数量

    std::vector<Slot> slots;                       // 箱子内部所有槽位

    Chest()
      : PlaceableItemBase(),
        slots(static_cast<std::size_t>(CAPACITY)) {}

    // 返回箱子在世界坐标中的占用矩形（用于放置/渲染）。
    cocos2d::Rect placeRect() const override;
    // 返回用于碰撞检测的矩形（通常只使用下半部分，避免头顶挡路）。
    cocos2d::Rect collisionRect() const override;
    // 每个地图区域允许存在的最大箱子数量。
    int maxPerArea() const override { return MAX_PER_AREA; }
};

// 工具函数：基于 Chest 实例计算其占用矩形，便于调用方统一使用接口。
cocos2d::Rect chestRect(const Chest& chest);
// 工具函数：基于 Chest 实例计算其碰撞矩形。
cocos2d::Rect chestCollisionRect(const Chest& chest);
// 判断玩家在 worldPos 附近是否“接近任意箱子”，由 ChestController/IMapController 等调用。
bool isNearAnyChest(const cocos2d::Vec2& playerWorldPos, const std::vector<Chest>& chests);

} // namespace Game
