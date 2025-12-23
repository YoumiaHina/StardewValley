#pragma once

#include "cocos2d.h"
#include <memory>

namespace Game {
class Inventory;
}

namespace Controllers {

class IMapController;
class UIController;

// PlaceableItemSystemBase：所有“可放置物系统”（箱子系统、熔炉系统等）的抽象基类。
// - 职责：
//   1. 管理一个 DrawNode，用于绘制该系统下所有物体的可视占位（精灵或调试矩形）。
//   2. 提供统一的交互入口 InteractWithItem：先判断是否放置新物体，否则尝试与已有物体交互。
//   3. 把“是否可以放置/如何放置/如何交互”的具体规则交给子类实现（纯虚函数）。
// - 使用方式：
//   - 子类继承后实现 shouldPlace/tryPlace/tryInteractExisting 三个虚函数。
//   - 外部只需调用 InteractWithItem，不必关心放置还是交互，逻辑由基类统一调度。
class PlaceableItemSystemBase {
public:
    // 构造函数：初始化父节点与绘制节点指针为空。
    PlaceableItemSystemBase()
    : _parentNode(nullptr)
    , _drawNode(nullptr) {}

    // 虚析构：确保通过基类指针删除子类对象时，能调用到子类析构释放资源。
    virtual ~PlaceableItemSystemBase() = default;

    // 挂接到父节点：
    // - parentNode：场景中的某个 Node，作为系统 DrawNode 的父节点。
    // - zOrder    ：在父节点中的渲染顺序，数值越大越靠前。
    // 逻辑：
    //   1. 记录 _parentNode。
    //   2. 创建一个 DrawNode，并作为子节点添加到 parentNode 上。
    void attachTo(cocos2d::Node* parentNode, int zOrder) {
        _parentNode = parentNode;
        if (!_parentNode) return;
        _drawNode = cocos2d::DrawNode::create();
        _parentNode->addChild(_drawNode, zOrder);
    }

    // 统一的交互入口：
    // - map            ：当前地图控制器，用于坐标/边界判断。
    // - ui             ：UI 控制器，用于弹出提示与刷新 HUD/Hotbar。
    // - inventory      ：玩家背包（可能为空指针）。
    // - playerWorldPos ：玩家世界坐标。
    // - lastDir        ：玩家最近一次移动方向（用于确定前方格子）。
    // 流程：
    //   1. 若 map 或 ui 为空，直接返回 false。
    //   2. 若背包存在且 shouldPlace 返回 true，则优先尝试放置（调用子类的 tryPlace）。
    //      - 放置成功则返回 true。
    //   3. 若没有放置或放置失败，则调用 tryInteractExisting 让子类与已有物体交互。
    bool InteractWithItem(Controllers::IMapController* map,
                          Controllers::UIController* ui,
                          const std::shared_ptr<Game::Inventory>& inventory,
                          const cocos2d::Vec2& playerWorldPos,
                          const cocos2d::Vec2& lastDir) {
        if (!map || !ui) return false;
        if (inventory && shouldPlace(*inventory)) {
            bool placed = tryPlace(map, ui, inventory, playerWorldPos, lastDir);
            if (placed) return true;
        }
        return tryInteractExisting(map, ui, inventory, playerWorldPos, lastDir);
    }

protected:
    // 判定“当前背包选中格是否应该进入放置流程”的规则，由子类实现。
    // 例如：ChestController 需要选中的是箱子物品且数量>0，FurnaceController 则是熔炉物品。
    virtual bool shouldPlace(const Game::Inventory& inventory) const = 0;

    // 尝试放置新物体：子类实现具体放置规则。
    // - 可能会检查地图类型、碰撞、数量上限，并在成功时从背包扣除物品。
    virtual bool tryPlace(Controllers::IMapController* map,
                          Controllers::UIController* ui,
                          const std::shared_ptr<Game::Inventory>& inventory,
                          const cocos2d::Vec2& playerWorldPos,
                          const cocos2d::Vec2& lastDir) = 0;

    // 尝试与已有物体交互：子类根据玩家位置选出目标物体并执行操作。
    // 例如：打开箱子、向熔炉投入矿石/燃料等。
    virtual bool tryInteractExisting(Controllers::IMapController* map,
                                     Controllers::UIController* ui,
                                     const std::shared_ptr<Game::Inventory>& inventory,
                                     const cocos2d::Vec2& playerWorldPos,
                                     const cocos2d::Vec2& lastDir) = 0;

    // 系统挂载的父节点（一般是场景或 worldNode 的子节点）。
    cocos2d::Node* _parentNode;
    // 用于绘制所有可放置物体的 DrawNode（子类在 refreshVisuals 时使用）。
    cocos2d::DrawNode* _drawNode;
};

}
