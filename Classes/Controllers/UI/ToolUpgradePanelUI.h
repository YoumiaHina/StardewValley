#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include <functional>
#include "Game/Inventory.h"

namespace Controllers {

// 工具升级面板 UI：
// - 负责在画面中心显示一个简单的升级界面；
// - 展示每种工具当前等级、升级所需的金钱与材料；
// - 当玩家点击按钮时，调用 ToolUpgradeSystem 尝试进行升级。
// 注意：这里只做 UI 相关逻辑，不直接修改背包或 WorldState，
// 实际的升级判定和资源扣除由系统层 ToolUpgradeSystem 完成。
class ToolUpgradePanelUI {
public:
    // 构造函数：
    // - 需要一个当前场景指针，用于把面板节点挂到场景里；
    // - 以及一个玩家背包指针，用于查询有哪些工具可以升级。
    ToolUpgradePanelUI(cocos2d::Scene* scene,
                       std::shared_ptr<Game::Inventory> inv)
      : _scene(scene), _inventory(std::move(inv)) {}

    // 构建升级面板的所有 Cocos 节点（背景矩形、文字、按钮等）。
    // 若已经构建过，则直接返回，避免重复创建。
    void buildPanel();
    // 根据当前背包/全局箱子中的工具状态刷新界面显示：
    // - 更新每行工具等级、按钮文字；
    // - 显示/隐藏对应的材料图标。
    void refreshPanel();
    // 打开或关闭面板：
    // - show = true 时，先刷新数据再设置为可见；
    // - show = false 时，仅隐藏节点。
    void togglePanel(bool show);
    // 查询面板当前是否处于可见状态。
    bool isVisible() const;

    // 设置“升级成功后”的回调函数，例如用来刷新热键栏显示。
    // 使用 std::function<void()>，方便调用处传入 lambda 或普通函数。
    void setOnUpgraded(const std::function<void()>& cb);

private:
    // 每一行对应一种工具的 UI 控件集合。
    struct RowWidgets {
        cocos2d::Sprite* toolIcon = nullptr;
        cocos2d::Label* levelLabel = nullptr;
        cocos2d::Label* buttonLabel = nullptr;
        Game::ToolKind kind = Game::ToolKind::Axe;
        bool canUpgrade = false;
        std::vector<cocos2d::Sprite*> materialIcons;
    };
    // 所属场景指针，仅用于把面板节点挂到场景中。
    cocos2d::Scene* _scene = nullptr;
    // 持有一份玩家背包的 shared_ptr，用于查询工具与扣除材料。
    std::shared_ptr<Game::Inventory> _inventory;
    // 面板根节点：buildPanel 时创建，togglePanel 时统一设置可见性。
    cocos2d::Node* _panelNode = nullptr;
    // 存放每一行对应的控件指针，便于在 refreshPanel 中统一更新。
    std::vector<RowWidgets> _rows;
    // 升级成功时触发的回调函数（可为空）。
    std::function<void()> _onUpgraded;
};

}
