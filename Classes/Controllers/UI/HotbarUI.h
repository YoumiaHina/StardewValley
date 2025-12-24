#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include <string>
#include "Game/Inventory.h"

namespace Controllers {

// HotbarUI：
// - 作用：构建并刷新热键栏（物品图标/数量/选中高亮），并处理点击与滚轮切换。
// - 职责边界：只做 UI 呈现与输入命中检测，不实现背包规则；物品数据来自 Inventory。
// - 主要协作对象：UIController 负责统一调度；Inventory 提供槽位物品与数量；输入事件由 Scene/Controller 转发。
class HotbarUI {
public:
    // 构造：绑定场景与背包引用；节点在 buildHotbar 中创建。
    HotbarUI(cocos2d::Scene* scene,
             std::shared_ptr<Game::Inventory> inventory)
    : _scene(scene), _inventory(std::move(inventory)) {}

    // 设置背包背景贴图路径（用于热键栏背景显示）。
    void setInventoryBackground(const std::string& path);
    // 构建热键栏节点（只创建一次，后续复用）。
    void buildHotbar();
    // 刷新热键栏内容（图标/数量/选中提示）。
    void refreshHotbar();
    // 选中指定热键栏索引并刷新高亮。
    void selectHotbarIndex(int idx);
    // 处理鼠标按下事件：用于点击切换热键栏/选中格子。
    bool handleHotbarMouseDown(cocos2d::EventMouse* e);
    // 判断屏幕坐标是否命中热键栏区域（用于外部统一的输入路由）。
    bool handleHotbarAtPoint(const cocos2d::Vec2& screenPoint);
    // 处理滚轮：按方向切换选中的热键栏索引。
    void handleHotbarScroll(float dy);

    // 获取热键栏缩放。
    float getScale() const { return _hotbarScale; }
    // 设置热键栏缩放。
    void setScale(float s) { _hotbarScale = s; }

private:
    cocos2d::Scene* _scene = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;

    cocos2d::Node* _hotbarNode = nullptr;
    cocos2d::DrawNode* _hotbarHighlight = nullptr;
    std::vector<cocos2d::Label*> _hotbarLabels;
    std::vector<cocos2d::Sprite*> _hotbarIcons;
    std::vector<cocos2d::Label*> _hotbarQtyLabels;
    cocos2d::Label* _selectedHintLabel = nullptr;
    cocos2d::Sprite* _hotbarBgSprite = nullptr;
    std::string _inventoryBgPath;
    float _hotbarScale = 1.0f;
};

}
