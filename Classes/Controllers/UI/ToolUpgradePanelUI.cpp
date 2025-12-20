#include "Controllers/UI/ToolUpgradePanelUI.h"
#include "Controllers/Systems/ToolUpgradeSystem.h"

using namespace cocos2d;

namespace Controllers {

namespace {

const float UPGRADE_UI_SCALE = 1.2f;

std::string toolDisplayName(Game::ToolKind kind) {
    switch (kind) {
        case Game::ToolKind::Axe:      return "Axe";
        case Game::ToolKind::Hoe:      return "Hoe";
        case Game::ToolKind::Pickaxe:  return "Pickaxe";
        case Game::ToolKind::WaterCan: return "WaterCan";
        default: break;
    }
    return "";
}

std::string levelText(int level) {
    if (level <= 0) return "Lv 0 (Basic)";
    if (level == 1) return "Lv 1 (Copper)";
    if (level == 2) return "Lv 2 (Iron)";
    return "Lv 3 (Gold)";
}

}

void ToolUpgradePanelUI::buildPanel() {
    if (_panelNode) return;
    _panelNode = Node::create();
    if (_scene) {
        _scene->addChild(_panelNode, 6);
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();
        _panelNode->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                     origin.y + visibleSize.height / 2));
    }
    _panelNode->setVisible(false);

    auto bg = DrawNode::create();
    float w = 420.f * UPGRADE_UI_SCALE;
    float h = 220.f * UPGRADE_UI_SCALE;
    Vec2 v[4] = { Vec2(-w/2, -h/2), Vec2(w/2, -h/2), Vec2(w/2, h/2), Vec2(-w/2, h/2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f, 0.f, 0.f, 0.85f));
    _panelNode->addChild(bg);

    auto title = Label::createWithTTF("Tool Upgrade (U)", "fonts/arial.ttf", 22 * UPGRADE_UI_SCALE);
    if (title) {
        title->setPosition(Vec2(0, h/2 - 26 * UPGRADE_UI_SCALE));
        _panelNode->addChild(title);
    }

    _rows.clear();
    Game::ToolKind kinds[4] = {
        Game::ToolKind::Hoe,
        Game::ToolKind::Pickaxe,
        Game::ToolKind::Axe,
        Game::ToolKind::WaterCan
    };
    float startY = h/2 - 70 * UPGRADE_UI_SCALE;
    float gapY = 40 * UPGRADE_UI_SCALE;
    for (int i = 0; i < 4; ++i) {
        float y = startY - i * gapY;
        RowWidgets row;
        row.kind = kinds[i];

        auto nameLabel = Label::createWithTTF(toolDisplayName(kinds[i]), "fonts/arial.ttf", 18 * UPGRADE_UI_SCALE);
        if (nameLabel) {
            nameLabel->setAnchorPoint(Vec2(0.f, 0.5f));
            nameLabel->setPosition(Vec2(-w/2 + 20 * UPGRADE_UI_SCALE, y));
            _panelNode->addChild(nameLabel);
        }
        row.nameLabel = nameLabel;

        auto levelLabel = Label::createWithTTF("", "fonts/arial.ttf", 18 * UPGRADE_UI_SCALE);
        if (levelLabel) {
            levelLabel->setAnchorPoint(Vec2(0.f, 0.5f));
            levelLabel->setPosition(Vec2(-w/2 + 160 * UPGRADE_UI_SCALE, y));
            _panelNode->addChild(levelLabel);
        }
        row.levelLabel = levelLabel;

        auto buttonLabel = Label::createWithTTF("[Upgrade]", "fonts/arial.ttf", 18 * UPGRADE_UI_SCALE);
        if (buttonLabel) {
            buttonLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
            buttonLabel->setPosition(Vec2(w/2 - 70 * UPGRADE_UI_SCALE, y));
            buttonLabel->setColor(Color3B::YELLOW);
            _panelNode->addChild(buttonLabel);

            auto listener = EventListenerTouchOneByOne::create();
            listener->setSwallowTouches(true);
            listener->onTouchBegan = [buttonLabel](Touch* t, Event* e) {
                auto target = static_cast<Label*>(e->getCurrentTarget());
                Vec2 p = target->convertToNodeSpace(t->getLocation());
                Size s = target->getContentSize();
                Rect r(0, 0, s.width, s.height);
                if (r.containsPoint(p)) {
                    target->setScale(0.9f);
                    return true;
                }
                return false;
            };
            listener->onTouchEnded = [this, kinds, i, buttonLabel](Touch* t, Event* e) {
                buttonLabel->setScale(1.0f);
                bool ok = ToolUpgradeSystem::getInstance().upgradeToolOnce(_inventory, kinds[i]);
                if (ok) {
                    refreshPanel();
                    if (_onUpgraded) {
                        _onUpgraded();
                    }
                }
            };
            _panelNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, buttonLabel);
        }
        row.buttonLabel = buttonLabel;

        _rows.push_back(row);
    }
}

void ToolUpgradePanelUI::refreshPanel() {
    if (!_panelNode) return;
    for (auto& row : _rows) {
        int lv = ToolUpgradeSystem::getInstance().toolLevel(_inventory, row.kind);
        if (row.levelLabel) {
            row.levelLabel->setString(levelText(lv));
        }
    }
}

void ToolUpgradePanelUI::togglePanel(bool show) {
    if (!_panelNode) buildPanel();
    if (!_panelNode) return;
    if (show) {
        refreshPanel();
        _panelNode->setVisible(true);
    } else {
        _panelNode->setVisible(false);
    }
}

bool ToolUpgradePanelUI::isVisible() const {
    return _panelNode && _panelNode->isVisible();
}

void ToolUpgradePanelUI::setOnUpgraded(const std::function<void()>& cb) {
    _onUpgraded = cb;
}

}
