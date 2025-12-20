#include "Controllers/UI/ToolUpgradePanelUI.h"
#include "Controllers/Systems/ToolUpgradeSystem.h"
#include "Game/WorldState.h"

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
    float w = 420.f * UPGRADE_UI_SCALE * 2.0f;
    float h = 220.f * UPGRADE_UI_SCALE * 2.0f;
    Vec2 v[4] = { Vec2(-w/2, -h/2), Vec2(w/2, -h/2), Vec2(w/2, h/2), Vec2(-w/2, h/2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f, 0.f, 0.f, 0.85f));
    _panelNode->addChild(bg);

    auto title = Label::createWithTTF("Tool Upgrade", "fonts/arial.ttf", 22 * UPGRADE_UI_SCALE);
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
    float startY = h/2 - 90 * UPGRADE_UI_SCALE;
    float gapY = 70 * UPGRADE_UI_SCALE;
    for (int i = 0; i < 4; ++i) {
        float y = startY - i * gapY;
        RowWidgets row;
        row.kind = kinds[i];

        auto nameLabel = Label::createWithTTF(toolDisplayName(kinds[i]), "fonts/arial.ttf", 18 * UPGRADE_UI_SCALE);
        if (nameLabel) {
            nameLabel->setAnchorPoint(Vec2(0.f, 0.5f));
            nameLabel->setPosition(Vec2(-w/2 + 40 * UPGRADE_UI_SCALE, y));
            _panelNode->addChild(nameLabel);
        }
        row.nameLabel = nameLabel;

        auto levelLabel = Label::createWithTTF("", "fonts/arial.ttf", 18 * UPGRADE_UI_SCALE);
        if (levelLabel) {
            levelLabel->setAnchorPoint(Vec2(0.f, 0.5f));
            levelLabel->setPosition(Vec2(-w/2 + 220 * UPGRADE_UI_SCALE, y));
            _panelNode->addChild(levelLabel);
        }
        row.levelLabel = levelLabel;

        float iconStartX = -w/2 + 380.f * UPGRADE_UI_SCALE;
        float iconGapX = 30.f * UPGRADE_UI_SCALE;
        for (int j = 0; j < 5; ++j) {
            auto icon = Sprite::create();
            if (icon) {
                icon->setAnchorPoint(Vec2(0.5f, 0.5f));
                icon->setPosition(Vec2(iconStartX + iconGapX * static_cast<float>(j), y));
                icon->setVisible(false);
                _panelNode->addChild(icon);
                row.materialIcons.push_back(icon);
            }
        }

        auto buttonLabel = Label::createWithTTF("[Upgrade]", "fonts/arial.ttf", 18 * UPGRADE_UI_SCALE);
        if (buttonLabel) {
            buttonLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
            buttonLabel->setPosition(Vec2(w/2 - 90 * UPGRADE_UI_SCALE, y));
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
                if (i < 0 || i >= static_cast<int>(_rows.size())) {
                    return;
                }
                if (!_rows[static_cast<std::size_t>(i)].canUpgrade) {
                    return;
                }
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
        row.canUpgrade = false;
        bool hasTool = false;
        int lv = 0;
        if (_inventory) {
            std::size_t sz = _inventory->size();
            for (std::size_t i = 0; i < sz; ++i) {
                const Game::ToolBase* t = _inventory->toolAt(i);
                if (t && t->kind() == row.kind) {
                    hasTool = true;
                    lv = t->level();
                    break;
                }
            }
            if (!hasTool) {
                auto& ws = Game::globalState();
                const Game::Inventory* wsInv = ws.inventory.get();
                if (_inventory.get() == wsInv) {
                    for (const auto& s : ws.globalChest.slots) {
                        if (s.kind == Game::SlotKind::Tool && s.tool && s.tool->kind() == row.kind) {
                            hasTool = true;
                            lv = s.tool->level();
                            break;
                        }
                    }
                }
            }
        }
        if (!hasTool) {
            if (row.levelLabel) {
                row.levelLabel->setString("No Tool");
            }
            if (row.buttonLabel) {
                row.buttonLabel->setString("[No Tool]");
                row.buttonLabel->setColor(Color3B(150, 150, 150));
            }
            for (auto* icon : row.materialIcons) {
                if (icon) icon->setVisible(false);
            }
            continue;
        }
        if (row.levelLabel) {
            row.levelLabel->setString(levelText(lv));
        }
        long long goldCost = 0;
        Game::ItemType materialType = Game::ItemType::CopperIngot;
        int materialQty = 0;
        bool affordable = false;
        bool hasNext = ToolUpgradeSystem::getInstance().nextUpgradeCost(_inventory,
                                                                        row.kind,
                                                                        goldCost,
                                                                        materialType,
                                                                        materialQty,
                                                                        affordable);
        row.canUpgrade = hasNext && affordable;
        if (row.buttonLabel) {
            if (!hasNext) {
                row.buttonLabel->setString("[Max]");
                row.buttonLabel->setColor(Color3B(150, 150, 150));
            } else {
                std::string text = StringUtils::format("[Upgrade %lldG]", goldCost);
                row.buttonLabel->setString(text);
                if (row.canUpgrade) {
                    row.buttonLabel->setColor(Color3B::YELLOW);
                } else {
                    row.buttonLabel->setColor(Color3B(150, 150, 150));
                }
            }
        }
        std::string iconPath;
        if (hasNext) {
            switch (materialType) {
                case Game::ItemType::CopperIngot: iconPath = "Mineral/copperIngot.png"; break;
                case Game::ItemType::IronIngot:   iconPath = "Mineral/ironIngot.png"; break;
                case Game::ItemType::GoldIngot:   iconPath = "Mineral/goldIngot.png"; break;
                default: break;
            }
        }
        for (std::size_t idx = 0; idx < row.materialIcons.size(); ++idx) {
            auto* icon = row.materialIcons[idx];
            if (!icon) continue;
            if (!hasNext || iconPath.empty() || idx >= static_cast<std::size_t>(materialQty)) {
                icon->setVisible(false);
                continue;
            }
            icon->setTexture(iconPath);
            if (icon->getTexture()) {
                Size cs = icon->getContentSize();
                float target = 26.f * UPGRADE_UI_SCALE;
                float sx = cs.width > 0 ? target / cs.width : 1.0f;
                float sy = cs.height > 0 ? target / cs.height : 1.0f;
                float s = std::min(sx, sy);
                icon->setScale(s);
                icon->setVisible(true);
            } else {
                icon->setVisible(false);
            }
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
