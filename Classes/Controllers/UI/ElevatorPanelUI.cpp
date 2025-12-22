#include "Controllers/UI/ElevatorPanelUI.h"

using namespace cocos2d;
using namespace cocos2d::ui;

namespace Controllers {

void ElevatorPanelUI::buildPanel() {
    if (_panel) return;
    _panel = Layout::create();
    _panel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    _panel->setBackGroundColor(Color3B(20,20,20));
    _panel->setBackGroundColorOpacity(180);
    Size vs = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    _panel->setContentSize(Size(960, 400));
    _panel->setAnchorPoint(Vec2(0.5f, 0.5f));
    _panel->setPosition(origin + Vec2(vs.width/2, vs.height/2));
    _panel->setVisible(false);
    if (_scene) {
        _scene->addChild(_panel, 1000);
    }
}

void ElevatorPanelUI::refreshButtons(const std::vector<int>& floors) {
    if (!_panel) return;
    _panel->removeAllChildren();
    int cols = 8;
    float cell = 80.0f;
    float padding = 16.0f;
    float w = _panel->getContentSize().width;
    float h = _panel->getContentSize().height;
    auto title = Label::createWithTTF("Elevator", "fonts/arial.ttf", 26);
    if (title) {
        title->setPosition(Vec2(w/2, h - 28));
        _panel->addChild(title);
    }
    float totalWidth = cols * cell + (cols - 1) * padding;
    float startX = (w - totalWidth) * 0.5f + cell * 0.5f;
    int i = 0;
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
        btn->setTitleText(StringUtils::format("%d", f));
        btn->setTitleFontName("fonts/arial.ttf");
        btn->setTitleFontSize(22);
        btn->setContentSize(Size(cell, cell));
        btn->setScale9Enabled(true);
        btn->setColor(Color3B(240, 200, 120));
        btn->setPosition(Vec2(x, y));
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
    if (!_panel) buildPanel();
    if (!_panel) return;
    _panel->setVisible(visible);
}

bool ElevatorPanelUI::isVisible() const {
    return _panel && _panel->isVisible();
}

}

