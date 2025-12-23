#pragma once

#include <vector>
#include "cocos2d.h"
#include "Game/Animal.h"
#include "Controllers/Map/IMapController.h"

namespace Controllers {

// 动物系统（唯一来源）：
// - 负责农场动物的运行时状态列表（位置/目标/成长/喂食）与持久化同步到 WorldState。
// - 负责动物精灵节点的创建、挂载、位置更新、排序与销毁时机管理。
// - 协作对象：IMapController 提供瓦片/碰撞/挂载入口与掉落生成；UI/场景通过该系统发起购买/喂食/每日推进。
class AnimalSystem {
public:
    // 构造：从 WorldState 还原动物列表，并绑定地图与世界节点用于可视化挂载。
    // - map/worldNode 为空时系统仍可存在，但无法生成精灵与交互（接口会早退）。
    AnimalSystem(Controllers::IMapController* map, cocos2d::Node* worldNode);

    // 每帧更新：驱动动物游走、同步精灵位置与状态标签，并回写 WorldState。
    // - dt 为秒；内部速度单位与地图坐标系一致。
    void update(float dt);

    // 尝试喂食：在玩家附近选取可交互动物，满足饲料规则则消耗物品并标记 fedToday。
    // - consumedQty 为建议消耗数量（当前固定为 1）；背包扣除由上层执行以避免系统直接耦合 Inventory。
    bool tryFeedAnimal(const cocos2d::Vec2& playerPos, Game::ItemType feedType, int& consumedQty);

    // 生成动物：在指定位置创建运行时实例并挂接可视化，写回 WorldState。
    // - pos 为世界坐标；系统内部会直接写入 Animal.pos/target。
    void spawnAnimal(Game::AnimalType type, const cocos2d::Vec2& pos);

    // 购买动物：扣除金币后生成动物；price 由调用方决定（通常来自 Game::animalPrice）。
    bool buyAnimal(Game::AnimalType type, const cocos2d::Vec2& pos, long long price);

private:
    // 动物实例：运行时状态 + 可视化节点引用（由系统统一管理生命周期）。
    struct Instance {
        Game::Animal animal; // 动物运行时状态（唯一来源：由系统推进并写回 WorldState）
        cocos2d::Sprite* sprite = nullptr; // 动物可视精灵节点（由系统创建/挂载/排序）
        cocos2d::Label* growthLabel = nullptr; // 头顶状态文本（成年/剩余天数 + 喂食状态）
        float idleTimer = 0.0f; // 停留倒计时（>0 时本帧不重新选目标）
    };

    Controllers::IMapController* _map = nullptr; // 地图接口（瓦片、挂载、排序、碰撞等）
    cocos2d::Node* _worldNode = nullptr; // 世界节点挂载点（精灵节点统一挂在其下）
    std::vector<Instance> _animals; // 动物实例列表（系统唯一维护）

    // 将运行时动物列表写回 WorldState（用于存档与跨系统读取）。
    void syncSave();

    // 确保实例拥有精灵与状态标签，并完成挂载与缩放。
    void ensureSprite(Instance& inst);
    // 刷新实例头顶文本（Adult/剩余天数 + Full/Hungry）并更新显示位置。
    void updateGrowthLabel(Instance& inst);
};

// 每日推进：推进动物成长与产物；若 map 为 Farm 则生成地图掉落，否则写入 farmDrops。
void advanceAnimalsDaily(Controllers::IMapController* map);

}
