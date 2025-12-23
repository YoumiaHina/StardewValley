#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include "Controllers/Systems/PlaceableItemSystemBase.h"
#include "Game/Furnace.h"
#include "Game/Inventory.h"

namespace Controllers {

class IMapController;
class UIController;

// FurnaceController：熔炉系统控制器，负责管理当前地图上的所有熔炉状态与交互。
// - 职责：
//   1. 维护运行时熔炉列表（_runtime，指向 WorldState 中对应容器）。
//   2. 负责熔炉的放置规则、交互判定（投入矿石/燃料）、计时更新与锭掉落。
//   3. 驱动熔炉的视觉刷新（冷却/加热贴图切换）。
// - 协作对象：
//   - IMapController：提供地图坐标转换、掉落生成、边界/碰撞信息。
//   - UIController：用于弹出提示文字、刷新背包 Hotbar。
//   - Game::Inventory：读取/扣除熔炼所需矿石与燃料。
class FurnaceController : public PlaceableItemSystemBase {
public:
    // 构造：初始化内部指针，具体容器在 bindContext/syncLoad 时绑定。
    FurnaceController();

    // 挂接到父节点：由基类创建 DrawNode，并作为熔炉精灵的父节点。
    void attachTo(cocos2d::Node* parentNode, int zOrder);

    // 绑定上下文：指定当前地图/UI/背包，后续交互与更新均依赖这些指针。
    void bindContext(Controllers::IMapController* map,
                     Controllers::UIController* ui,
                     std::shared_ptr<Game::Inventory> inventory);

    // 从 WorldState 同步当前地图的熔炉列表，并修正缺失的 dropOffset。
    void syncLoad();
    // 每帧更新：推进各熔炉 remainingSeconds，完成时生成掉落并弹出提示。
    void update(float dt);
    // 重新绘制所有熔炉的精灵（冷/热两套贴图）。
    void refreshVisuals();

    // 只读访问当前地图上的熔炉列表。
    const std::vector<Game::Furnace>& furnaces() const;
    // 可写访问当前地图上的熔炉列表。
    std::vector<Game::Furnace>& furnaces();

    // 玩家在 worldPos 处尝试与熔炉交互（放置或投入矿石/燃料）。
    bool interactAt(const cocos2d::Vec2& playerWorldPos,
                    const cocos2d::Vec2& lastDir);
    // 判断给定世界坐标附近是否存在熔炉（用于工具/移动避让）。
    bool isNearFurnace(const cocos2d::Vec2& worldPos) const;
    // 判定给定世界坐标是否与任一熔炉发生碰撞。
    bool collides(const cocos2d::Vec2& worldPos) const;

private:
    // 判断当前背包选中格是否为“熔炉物品”，用于驱动放置逻辑。
    bool shouldPlace(const Game::Inventory& inventory) const override;

    // 尝试在地图上放置新熔炉：选择中心点、做碰撞/数量校验并扣除物品。
    bool tryPlace(Controllers::IMapController* map,
                  Controllers::UIController* ui,
                  const std::shared_ptr<Game::Inventory>& inventory,
                  const cocos2d::Vec2& playerWorldPos,
                  const cocos2d::Vec2& lastDir) override;

    // 尝试对已有熔炉进行交互：投入矿石/燃料并启动计时。
    bool tryInteractExisting(Controllers::IMapController* map,
                             Controllers::UIController* ui,
                             const std::shared_ptr<Game::Inventory>& inventory,
                             const cocos2d::Vec2& playerWorldPos,
                             const cocos2d::Vec2& lastDir) override;

    // 在视野内查找距离玩家最近的熔炉索引。
    int findNearestFurnace(const cocos2d::Vec2& playerWorldPos, float maxDist) const;

    // 当前所在地图控制器（不拥有所有权）。
    Controllers::IMapController* _map = nullptr;
    // UI 控制器：用于弹出文字与刷新 Hotbar。
    Controllers::UIController* _ui = nullptr;
    // 玩家背包共享指针：用于扣除熔炼所需物品。
    std::shared_ptr<Game::Inventory> _inventory;
    // 当前地图对应的熔炉列表指针（指向 WorldState 中的容器）。
    std::vector<Game::Furnace>* _runtime = nullptr;
};

}
