#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include <functional>
#include "Game/Inventory.h"
#include "Game/Recipe/RecipeBook.h"
#include "Controllers/Crafting/CraftingController.h"

namespace Controllers {

// CraftPanelUI：合成面板 UI。
// - 职责：创建/刷新合成列表与筛选入口，并转发点击事件到 CraftingController 执行合成。
// - 协作对象：CraftingController 负责合成规则与背包变更；RecipeBook 提供配方列表；UIController 管理面板生命周期。
class CraftPanelUI {
public:
    // 构造：绑定场景与背包引用；节点在 buildCraftPanel 中创建。
    CraftPanelUI(cocos2d::Scene* scene, std::shared_ptr<Game::Inventory> inv)
        : _scene(scene), _inventory(std::move(inv)) {}
    // 构建面板节点（背景、标题、筛选 Tab、列表容器、分页按钮）。
    void buildCraftPanel();
    // 刷新面板内容（根据筛选与分页重绘列表）。
    void refreshCraftPanel();
    // 显示/隐藏面板；show=true 时会确保构建并刷新。
    void toggleCraftPanel(bool show);
    // 面板是否可见。
    bool isVisible() const;
    // 切换配方分类并刷新列表。
    void setCategory(Game::RecipeCategory cat);
    // 获取当前分类。
    Game::RecipeCategory category() const { return _category; }

    // 合成完成回调：参数表示是否成功。
    std::function<void(bool)> onCrafted;

private:
    // 重建当前分类下的配方列表。
    void rebuildRecipes();
    // 更新标题文本。
    void updateTitle();
    // 更新 Tab 的视觉状态。
    void updateTabsVisual();
    // 将材料列表格式化为字符串（包含拥有数量）。
    std::string formatIngredientsLine(const Game::RecipeBase& recipe) const;

    cocos2d::Scene* _scene = nullptr; // 所属场景（用于挂载 UI 节点）
    std::shared_ptr<Game::Inventory> _inventory; // 玩家背包（合成材料数量的唯一来源）
    cocos2d::Node* _panelNode = nullptr; // 面板根节点（显示/隐藏的 owner）
    cocos2d::Node* _listNode = nullptr; // 配方列表容器节点（每次刷新重建）
    cocos2d::Node* _tabsNode = nullptr; // 分类 Tab 容器节点
    cocos2d::Label* _titleLabel = nullptr; // 标题文本（显示当前分类/页码等）
    cocos2d::Label* _tabPlaceable = nullptr; // Placeable 分类 Tab 文本
    cocos2d::Label* _tabFood = nullptr; // Food 分类 Tab 文本

    std::unique_ptr<Controllers::CraftingController> _craftingController; // 合成控制器（负责扣材料与加产物）
    int _pageIndex = 0; // 当前页索引（从 0 开始）
    int _pageSize = 5; // 每页显示配方数量
    Game::RecipeCategory _category = Game::RecipeCategory::Placeable; // 当前筛选分类
    std::vector<std::shared_ptr<Game::RecipeBase>> _recipes; // 当前分类下的配方列表（用于分页展示）
};

} // namespace Controllers
