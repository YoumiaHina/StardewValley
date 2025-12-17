#include "Controllers/MineElevatorController.h"

using namespace cocos2d;
using namespace cocos2d::ui;

namespace Controllers {

void MineElevatorController::buildPanel() {
    if (_panel) return;
    _panel = Layout::create();
    _panel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    _panel->setBackGroundColor(Color3B(20,20,20));
    _panel->setBackGroundColorOpacity(180);
    // 中央面板
    Size vs = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    _panel->setContentSize(Size(360, 320));
    _panel->setAnchorPoint(Vec2(0.5f, 0.5f));
    _panel->setPosition(origin + Vec2(vs.width/2, vs.height/2));
    _panel->setVisible(false);

    _scene->addChild(_panel, 1000);
    _refreshButtons();
}

void MineElevatorController::togglePanel() {
    if (!_panel) buildPanel();
    // 仅允许在 0 层使用电梯
    if (_map && _map->currentFloor() > 0) { if (_panel) _panel->setVisible(false); return; }
    _refreshButtons();
    bool newVisible = !_panel->isVisible();
    _panel->setVisible(newVisible);
    if (_setMovementLocked) _setMovementLocked(newVisible);
}

void MineElevatorController::_refreshButtons() {
    if (!_panel) return;
    _panel->removeAllChildren();
    auto floors = _map->getActivatedElevatorFloors();
    // 小方块网格排布
    int cols = 4;
    float cell = 64.0f;
    float padding = 12.0f;
    float w = _panel->getContentSize().width;
    float h = _panel->getContentSize().height;
    // 标题
    auto title = Label::createWithTTF("Elevator", "fonts/arial.ttf", 22);
    title->setPosition(Vec2(w/2, h - 22));
    _panel->addChild(title);
    int i = 0;
    for (int f : floors) {
        int col = i % cols;
        int row = i / cols;
        float x = padding + cell/2 + col * (cell + padding);
        float y = h - (padding + cell/2 + row * (cell + padding) + 48.0f); // 留出标题空间
        auto btn = Button::create();
        btn->setTitleText(StringUtils::format("%d", f));
        btn->setTitleFontSize(20);
        btn->setContentSize(Size(cell, cell));
        btn->setScale9Enabled(true);
        btn->setColor(Color3B(240, 200, 120));
        btn->setPosition(Vec2(x, y));
        btn->addClickEventListener([this, f](Ref*){ _jumpToFloor(f); });
        _panel->addChild(btn);
        i++;
    }
}

void MineElevatorController::_jumpToFloor(int floor) {
    if (!_map) return;
    _map->setFloor(floor);
    if (_monsters) { _monsters->resetFloor(); }
    if (floor > 0) {
        if (_monsters) _monsters->generateInitialWave();
    }
    // 更新 UI 楼层显示与关闭面板、解锁移动
    if (_onFloorChanged) _onFloorChanged(floor);
    if (_panel) _panel->setVisible(false);
    if (_setMovementLocked) _setMovementLocked(false);
}

} // namespace Controllers
