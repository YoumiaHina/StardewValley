#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include <functional>
#include "Game/Inventory.h"
#include "Game/Chest.h"

namespace Controllers {

// ChestPanelUI：箱子面板 UI。
// - 职责：创建/布局箱子格子 UI、处理点击/键盘事件，并调用 ChestController 提供的转移接口。
// - 不负责任何箱子/背包业务规则，所有物品/工具转移逻辑由 ChestController 完成。
// - 协作对象：Inventory 提供当前背包状态；Game::Chest 提供箱子槽位数组；
//   UIController 负责管理面板生命周期与与其他 UI 的联动。
class ChestPanelUI {
public:
    // 构造：绑定场景和背包引用，真正的节点在 buildChestPanel 中创建。
    ChestPanelUI(cocos2d::Scene* scene, std::shared_ptr<Game::Inventory> inv)
      : _scene(scene), _inventory(std::move(inv)) {}
    // 构建面板根节点及键盘监听（只创建一次，后续复用）。
    void buildChestPanel();
    // 刷新箱子面板内容：根据传入的 Chest 重建格子布局与图标。
    void refreshChestPanel(Game::Chest* chest);
    // 设置背包变更回调：每次箱子与背包之间发生转移时触发（用于刷新热键栏等）。
    void setOnInventoryChanged(const std::function<void()>& cb);
    // 当玩家点击背包（热键栏）中的某个格子时调用：尝试将选中的箱子格内容转移到对应背包格。
    void onInventorySlotClicked(int invIndex);
    // 显示/隐藏箱子面板。
    void toggleChestPanel(bool show);
    // 当前箱子面板是否可见。
    bool isVisible() const;

private:
    cocos2d::Scene* _scene = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    cocos2d::Node* _panelNode = nullptr;
    cocos2d::EventListenerKeyboard* _escListener = nullptr;
    Game::Chest* _currentChest = nullptr;
    cocos2d::Node* _slotsRoot = nullptr;
    cocos2d::DrawNode* _highlightNode = nullptr;
    std::vector<cocos2d::Sprite*> _cellIcons;
    std::vector<cocos2d::Label*> _cellCountLabels;
    std::vector<cocos2d::Label*> _cellNameLabels;
    std::function<void()> _onInventoryChanged;
    int _selectedIndex = -1;
    bool _shiftDown = false;
};

} 
