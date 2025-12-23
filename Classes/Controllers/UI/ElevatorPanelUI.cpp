#include "Controllers/UI/ElevatorPanelUI.h"

using namespace cocos2d;
using namespace cocos2d::ui;

namespace Controllers {

void ElevatorPanelUI::buildPanel() {
    // 只创建一次 Layout，避免重复 new / addChild
    if (_panel) return;
    // 创建一个半透明的深色背景，用作电梯面板
    _panel = Layout::create();
    _panel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    _panel->setBackGroundColor(Color3B(20,20,20));
    _panel->setBackGroundColorOpacity(180);
    // 获取当前可见区域与原点，用来把面板放在屏幕中央
    Size vs = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    // 固定宽高的面板（这里选择 960x400，只是简单示例尺寸）
    _panel->setContentSize(Size(960, 400));
    _panel->setAnchorPoint(Vec2(0.5f, 0.5f));
    _panel->setPosition(origin + Vec2(vs.width/2, vs.height/2));
    // 初始为隐藏，等需要时再显示
    _panel->setVisible(false);
    // 挂到场景上，使其参与渲染；zOrder 设为较高值保证在前景
    if (_scene) {
        _scene->addChild(_panel, 1000);
    }
}

void ElevatorPanelUI::refreshButtons(const std::vector<int>& floors) {
    // 若面板还没构建则不处理
    if (!_panel) return;
    // 每次刷新前先清空上一次的按钮与标题
    _panel->removeAllChildren();
    // 简单的网格布局：固定列数、单元格尺寸与间距
    int cols = 8;
    float cell = 80.0f;
    float padding = 16.0f;
    float w = _panel->getContentSize().width;
    float h = _panel->getContentSize().height;
    // 面板标题：“Elevator”
    auto title = Label::createWithTTF("Elevator", "fonts/arial.ttf", 26);
    if (title) {
        title->setPosition(Vec2(w/2, h - 28));
        _panel->addChild(title);
    }
    // 计算按钮区域的起始 X，使网格整体居中
    float totalWidth = cols * cell + (cols - 1) * padding;
    float startX = (w - totalWidth) * 0.5f + cell * 0.5f;
    int i = 0;
    // 依次为每个楼层创建一个按钮
    for (int f : floors) {
        int col = i % cols;
        int row = i / cols;
        float x = startX + col * (cell + padding);
        float y = h - (padding + cell/2 + row * (cell + padding) + 48.0f);
        auto btn = Button::create();
        if (!btn) {
            ++i;
            continue;
        }
        // 按钮上显示具体楼层数字
        btn->setTitleText(StringUtils::format("%d", f));
        btn->setTitleFontName("fonts/arial.ttf");
        btn->setTitleFontSize(22);
        btn->setContentSize(Size(cell, cell));
        btn->setScale9Enabled(true);
        btn->setColor(Color3B(240, 200, 120));
        btn->setPosition(Vec2(x, y));
        // 点击事件：把当前楼层 f 通过回调通知给外部
        // 捕获列表 [this, f] 表示 lambda 内可以使用 this 指针与当前按钮楼层号
        btn->addClickEventListener([this, f](Ref*) {
            if (_onFloorSelected) {
                _onFloorSelected(f);
            }
        });
        _panel->addChild(btn);
        ++i;
    }
}

void ElevatorPanelUI::togglePanel(bool visible) {
    // 若还没创建面板，则先构建一次
    if (!_panel) buildPanel();
    if (!_panel) return;
    // 仅切换可见性，不销毁节点
    _panel->setVisible(visible);
}

bool ElevatorPanelUI::isVisible() const {
    // 只有在节点存在且被设为可见时才认为“面板可见”
    return _panel && _panel->isVisible();
}

}
