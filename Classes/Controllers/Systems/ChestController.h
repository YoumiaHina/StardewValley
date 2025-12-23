#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include "Controllers/Systems/PlaceableItemSystemBase.h"
#include "Game/Chest.h"
#include "Game/Inventory.h"

namespace Controllers {

class IMapController;
class UIController;
class RoomMapController;

// ChestController：管理农场/室内场景中的可放置箱子。
// - 职责：负责箱子状态的唯一来源（位置、内容）、放置规则判断与碰撞检测。
// - 视觉：通过 PlaceableItemSystemBase 提供的 DrawNode 绘制箱子占位或精灵。
// - 协作对象：IMapController 提供坐标/地图信息；UIController 弹出箱子面板；
//   WorldState 作为箱子列表的持久化存储；Inventory 提供当前选中物品/工具。
class ChestController : public PlaceableItemSystemBase {
public:
    // 构造：isFarm=true 时使用 WorldState::farmChests，否则使用 houseChests。
    explicit ChestController(bool isFarm)
    : PlaceableItemSystemBase()
    , _isFarm(isFarm) {}

    // 挂接到父节点，并创建内部 DrawNode（由基类负责）。
    void attachTo(cocos2d::Node* parentNode, int zOrder);
    // 从 WorldState 同步读取箱子列表到本地容器（进入场景时调用一次）。
    void syncLoad();

    // 只读访问当前场景箱子列表（用于地图控制器判断/绘制）。
    const std::vector<Game::Chest>& chests() const;
    // 可写访问箱子列表（仅系统内部或地图在刷新可视时使用）。
    std::vector<Game::Chest>& chests();

    // 判断指定世界坐标附近是否有箱子（用于交互提示）。
    bool isNearChest(const cocos2d::Vec2& worldPos) const;
    // 判断指定世界坐标是否与任意箱子碰撞（用于放置/走位阻挡）。
    bool collides(const cocos2d::Vec2& worldPos) const;

    // 根据当前箱子列表刷新调试/占位可视。
    void refreshVisuals();

private:
    // 是否应该进入“放置箱子”流程：当前选中格必须是箱子物品且数量>0。
    bool shouldPlace(const Game::Inventory& inventory) const override;

    // 放置箱子：根据地图类型转发到农场/室内/室外对应实现。
    bool tryPlace(Controllers::IMapController* map,
                  Controllers::UIController* ui,
                  const std::shared_ptr<Game::Inventory>& inventory,
                  const cocos2d::Vec2& playerWorldPos,
                  const cocos2d::Vec2& lastDir) override;

    // 交互已有箱子：找到最近箱子并打开 ChestPanelUI。
    bool tryInteractExisting(Controllers::IMapController* map,
                             Controllers::UIController* ui,
                             const std::shared_ptr<Game::Inventory>& inventory,
                             const cocos2d::Vec2& playerWorldPos,
                             const cocos2d::Vec2& lastDir) override;

    // 标记当前控制器对应的地图类型：
    // - true  时表示控制农场上的箱子（使用 WorldState::farmChests）。
    // - false 时表示控制室内箱子（使用 WorldState::houseChests）。
    bool _isFarm = true;
    // 当前场景的箱子列表副本：
    // - 进入场景时从 WorldState 同步到此处；
    // - 发生增删改时，再通过辅助函数写回 WorldState。
    std::vector<Game::Chest> _chests;
};

// 打开玩家附近的箱子：根据地图类型选择目标箱子，并弹出 ChestPanelUI。
bool openChestNearPlayer(Controllers::IMapController* map,
                         Controllers::UIController* ui,
                         const cocos2d::Vec2& playerWorldPos,
                         const cocos2d::Vec2& lastDir);

// 打开全局箱子（扩展背包）：直接使用 WorldState::globalChest。
bool openGlobalChest(Controllers::UIController* ui);

// 在农场放置箱子：基于玩家位置与朝向选择格子，并同步到 WorldState::farmChests。
bool placeChestOnFarm(Controllers::IMapController* map,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const cocos2d::Vec2& playerPos,
                      const cocos2d::Vec2& lastDir);

// 在室内放置箱子：避免门/床区域，并同步到 WorldState::houseChests。
bool placeChestInRoom(Controllers::RoomMapController* room,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const cocos2d::Vec2& playerPos);

// 室外（Town/Beach）当前不支持放置箱子，占位接口。
bool placeChestOnOutdoorMap(Controllers::IMapController* map,
                            Controllers::UIController* ui,
                            std::shared_ptr<Game::Inventory> inventory,
                            const cocos2d::Vec2& playerPos);

// 背包 -> 箱子 单步转移：根据当前选中背包格，把一个物品/工具塞入指定箱子格。
// - 会检查类型/栈上限/工具唯一性，并同步修改 WorldState 中对应箱子。
void transferChestCell(Game::Chest& chest,
                       int flatIndex,
                       Game::Inventory& inventory);

// 箱子 -> 背包：从指定箱子格转移到指定背包格。
// - moveAll=true 时尽量搬满（受栈上限与箱子余量限制）。
// - 成功时同步 WorldState 的箱子列表。
bool transferChestToInventory(Game::Chest& chest,
                              int flatIndex,
                              Game::Inventory& inventory,
                              int invIndex,
                              bool moveAll);

// 背包 -> 箱子：从当前选中的背包格向指定箱子格转移。
// - moveAll=true 时进行多次 single-step 转移，直至无法再转移。
// - 实际单步逻辑复用 transferChestCell。
bool transferInventoryToChest(Game::Chest& chest,
                              int flatIndex,
                              Game::Inventory& inventory,
                              bool moveAll);

}
