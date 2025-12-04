#include "Controllers/AbyssElevatorController.h"

using namespace cocos2d;
using namespace cocos2d::ui;

namespace Controllers {

void AbyssElevatorController::buildPanel() {
    if (_panel) return;
    _panel = Layout::create();
    _panel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    _panel->setBackGroundColor(Color3B(20,20,20));
    _panel->setBackGroundColorOpacity(180);
    _panel->setContentSize(Size(260, 360));
    _panel->setAnchorPoint(Vec2(1,1));
    _panel->setPosition(Vec2(Director::getInstance()->getVisibleSize().width - 20,
                              Director::getInstance()->getVisibleSize().height - 20));
    _panel->setVisible(false);

    _scene->addChild(_panel, 1000);
    _refreshButtons();
}

void AbyssElevatorController::togglePanel() {
    if (!_panel) buildPanel();
    _refreshButtons();
    _panel->setVisible(!_panel->isVisible());
}

void AbyssElevatorController::_refreshButtons() {
    if (!_panel) return;
    _panel->removeAllChildren();
    auto floors = _map->getActivatedElevatorFloors();
    float y = _panel->getContentSize().height - 40.0f;
    for (int f : floors) {
        auto btn = Button::create();
        btn->setTitleText(StringUtils::format("Floor %d", f));
        btn->setTitleFontSize(20);
        btn->setContentSize(Size(200, 36));
        btn->setScale9Enabled(true);
        btn->setColor(Color3B(240, 200, 120));
        btn->setPosition(Vec2(_panel->getContentSize().width/2, y));
        btn->addClickEventListener([this, f](Ref*){ _jumpToFloor(f); });
        _panel->addChild(btn);
        y -= 44.0f;
    }
}

void AbyssElevatorController::_jumpToFloor(int floor) {
    if (!_map) return;
    _map->setFloor(floor);
    if (_monsters) { _monsters->resetFloor(); _monsters->generateInitialWave(); }
    if (_mining) { _mining->generateNodesForFloor(); }
}

} // namespace Controllers