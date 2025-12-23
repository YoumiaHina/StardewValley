#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include <functional>
#include <utility>
#include "Controllers/Store/StoreController.h"
#include "Game/Item.h"
#include "Game/Animals/Animal.h"

namespace Game { class Inventory; }

namespace Controllers {

// 商店分类：用于切换列表内容。
enum class StoreCategory {
    Produce,
    Mineral
};

// 商店面板 UI：
// - 负责商店界面的创建、刷新与显示隐藏，不直接处理交易规则。
// - 协作对象：StoreController 负责买卖逻辑；Inventory 提供拥有数量；UIController 负责面板生命周期调度。
class StorePanelUI {
public:
    // 构造：绑定场景与背包引用；实际节点在 buildStorePanel 中创建。
    StorePanelUI(cocos2d::Scene* scene, std::shared_ptr<Game::Inventory> inv)
      : _scene(scene), _inventory(std::move(inv)) {}
    // 构建面板节点（背景、标题、列表容器、分页与分类切换）。
    void buildStorePanel();
    // 刷新面板内容（分页、名称、价格与买卖入口）。
    void refreshStorePanel();
    // 显示/隐藏面板；show=true 时会确保构建并刷新。
    void toggleStorePanel(bool show);
    // 切换分类并刷新列表。
    void setCategory(StoreCategory cat);
    // 当前分类。
    StoreCategory category() const { return _category; }
    // 面板是否处于可见状态。
    bool isVisible() const;
    // 买卖完成回调：参数表示是否成功。
    std::function<void(bool)> onPurchased;

private:
    // 重建商品列表（根据分类填充种子/农产品/矿物等条目）。
    void rebuildItems();
    // 刷新标题文本。
    void updateTitle();
    // 刷新分类 Tab 的视觉状态。
    void updateTabsVisual();

    cocos2d::Scene* _scene = nullptr; // 所属场景（用于挂载 UI 节点）
    std::shared_ptr<Game::Inventory> _inventory; // 玩家背包（用于显示拥有数量/执行交易）
    cocos2d::Node* _panelNode = nullptr; // 面板根节点（显示/隐藏的 owner）
    cocos2d::Node* _listNode = nullptr; // 商品列表容器节点（每次刷新重建内容）
    cocos2d::Label* _titleLabel = nullptr; // 标题文本（显示当前分类/页码等）
    cocos2d::Node* _tabsNode = nullptr; // 分类 Tab 容器节点
    cocos2d::Label* _produceTab = nullptr; // 农产品分类 Tab 文本
    cocos2d::Label* _mineralTab = nullptr; // 矿物分类 Tab 文本
    std::unique_ptr<StoreController> _storeController; // 交易控制器（负责买卖规则与背包/金币变更）
    int _pageIndex = 0; // 当前页索引（从 0 开始）
    int _pageSize = 5; // 每页显示条目数
    StoreCategory _category = StoreCategory::Produce; // 当前分类
    std::vector<Game::ItemType> _items; // 当前分类下的商品类型列表（用于分页展示）
};

// 动物商店面板 UI：
// - 仅负责动物购买界面的创建、刷新与显示隐藏。
// - 协作对象：onBuyAnimal 回调由上层（通常是场景/系统）提供以完成扣费与生成。
class AnimalStorePanelUI {
public:
    // 构造：绑定场景；实际节点在 buildAnimalStorePanel 中创建。
    explicit AnimalStorePanelUI(cocos2d::Scene* scene)
      : _scene(scene) {}
    // 构建动物商店面板节点（背景、列表容器等）。
    void buildAnimalStorePanel();
    // 刷新动物商店列表（名称、价格与购买入口）。
    void refreshAnimalStorePanel();
    // 显示/隐藏动物商店面板。
    void toggleAnimalStorePanel(bool show);
    // 面板是否处于可见状态。
    bool isVisible() const;
    // 购买回调：返回 true 表示购买成功（由上层扣费与生成动物）。
    std::function<bool(Game::AnimalType)> onBuyAnimal;
    // 购买完成回调：参数表示是否成功。
    std::function<void(bool)> onPurchased;

private:
    cocos2d::Scene* _scene = nullptr; // 所属场景（用于挂载 UI 节点）
    cocos2d::Node* _panelNode = nullptr; // 面板根节点（显示/隐藏的 owner）
    cocos2d::Node* _listNode = nullptr; // 动物列表容器节点
};

}
