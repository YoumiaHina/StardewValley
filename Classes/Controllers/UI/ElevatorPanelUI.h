/**
 * ElevatorPanelUI：矿洞电梯选择楼层的弹出面板。
 * 负责：在屏幕中央构建一个简单的 UI 面板，用按钮列出可达的楼层。
 * 协作对象：由 UIController 持有并调用 build / refresh / toggle；
 *          当玩家点击某一楼层按钮时，通过回调把“选中的楼层编号”通知外部。
 */
#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <vector>
#include <functional>

namespace Controllers {

class ElevatorPanelUI {
public:
    // 构造函数：记录场景指针，稍后在该场景中添加面板节点
    explicit ElevatorPanelUI(cocos2d::Scene* scene)
      : _scene(scene) {}

    // 构建电梯面板节点：只在首次调用时创建 Layout，之后复用
    void buildPanel();
    // 按照传入的楼层列表创建/刷新按钮，每个按钮代表一个可选楼层
    void refreshButtons(const std::vector<int>& floors);
    // 显示或隐藏面板；若尚未创建会自动调用 buildPanel
    void togglePanel(bool visible);
    // 查询面板当前是否可见
    bool isVisible() const;

    // 设置“楼层被点击时”的回调；参数为被选中的楼层号
    // 这里使用 std::function 是为了让外部可以传入 lambda / 成员函数绑定等任意可调用对象。
    void setOnFloorSelected(const std::function<void(int)>& cb) { _onFloorSelected = cb; }

private:
    // 所属场景指针：用于把面板节点挂到场景节点树上
    cocos2d::Scene* _scene = nullptr;
    // 电梯面板根节点（一个 Layout），负责承载标题和楼层按钮
    cocos2d::ui::Layout* _panel = nullptr;
    // 玩家点击某个楼层按钮时触发的回调
    std::function<void(int)> _onFloorSelected;
};

}
