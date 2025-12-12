#include "Controllers/UI/ChestPanelUI.h"
#include "Game/WorldState.h"
#include "Game/Item.h"

using namespace cocos2d;

namespace Controllers {

void ChestPanelUI::buildChestPanel() {
    if (_panelNode) return;
    _panelNode = Node::create();
    if (_scene) _scene->addChild(_panelNode, 5);
}

void ChestPanelUI::refreshChestPanel(const Game::Chest& chest) {
    if (!_panelNode) return;
    _panelNode->removeAllChildren();
    auto bg = DrawNode::create();
    float w = 360.f, h = 240.f;
    Vec2 v[4] = { Vec2(-w/2,-h/2), Vec2(w/2,-h/2), Vec2(w/2,h/2), Vec2(-w/2,h/2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f,0.f,0.f,0.55f));
    _panelNode->addChild(bg);
    auto title = Label::createWithTTF("Chest Storage", "fonts/Marker Felt.ttf", 20);
    title->setPosition(Vec2(0, h/2 - 26));
    _panelNode->addChild(title);
    auto list = Node::create();
    _panelNode->addChild(list);
    float startY = 60.f, gapY = 60.f;
    int i = 0;
    for (const auto &entry : chest.bag.all()) {
        auto t = entry.first; int have = entry.second; if (have <= 0) continue;
        float y = startY - i * gapY;
        auto nameLabel = Label::createWithTTF(Game::itemName(t), "fonts/Marker Felt.ttf", 18);
        nameLabel->setPosition(Vec2(-140, y));
        list->addChild(nameLabel);
        auto countLabel = Label::createWithTTF(StringUtils::format("x%d", have), "fonts/Marker Felt.ttf", 18);
        countLabel->setPosition(Vec2(-60, y));
        list->addChild(countLabel);
        ++i;
    }
}

void ChestPanelUI::toggleChestPanel(bool show) {
    if (_panelNode) _panelNode->setVisible(show);
}

}
