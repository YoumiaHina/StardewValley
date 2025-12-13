#include "Controllers/UI/HotbarUI.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Tool/ToolBase.h"
#include "ui/CocosGUI.h"

using namespace cocos2d;

namespace Controllers {

void HotbarUI::setInventoryBackground(const std::string& path) {
    _inventoryBgPath = path;
    if (_hotbarNode) {
        if (_hotbarBgSprite) {
            _hotbarBgSprite->removeFromParent();
            _hotbarBgSprite = nullptr;
        }
        _hotbarBgSprite = Sprite::create(_inventoryBgPath);
        if (_hotbarBgSprite) {
            _hotbarBgSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
            _hotbarNode->addChild(_hotbarBgSprite, 0);
            int slots = static_cast<int>(_inventory ? _inventory->size() : 0);
            float slotW = 80.0f * _hotbarScale, slotH = 32.0f * _hotbarScale, padding = 6.0f * _hotbarScale;
            float totalWidth = slots * slotW + (slots - 1) * padding;
            float targetW = totalWidth + 20.0f;
            float targetH = slotH + 16.0f;
            auto cs = _hotbarBgSprite->getContentSize();
            if (cs.width > 0 && cs.height > 0) {
                float sx = targetW / cs.width;
                float sy = targetH / cs.height;
                float s = std::min(sx, sy);
                _hotbarBgSprite->setScale(s);
            }
        }
    }
}

void HotbarUI::buildHotbar() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    if (_inventory) {
        int slotsPre = static_cast<int>(_inventory->size());
        for (int i = 0; i < slotsPre; ++i) {
            auto t = _inventory->toolAtMutable(i);
            if (t) { t->detachHotbarOverlay(); }
        }
    }
    _hotbarNode = Node::create();
    _hotbarNode->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 28));
    if (_scene) _scene->addChild(_hotbarNode, 2);

    int slots = static_cast<int>(_inventory ? _inventory->size() : 0);
    float slotW = 80.0f * _hotbarScale, slotH = 32.0f * _hotbarScale, padding = 6.0f * _hotbarScale;
    float totalWidth = slots * slotW + (slots - 1) * padding;
    bool useImageBg = false;
    if (!_inventoryBgPath.empty()) {
        _hotbarBgSprite = Sprite::create(_inventoryBgPath);
        if (_hotbarBgSprite) {
            _hotbarBgSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
            _hotbarNode->addChild(_hotbarBgSprite, 0);
            float targetW = totalWidth + 20.0f;
            float targetH = slotH + 16.0f;
            auto cs = _hotbarBgSprite->getContentSize();
            if (cs.width > 0 && cs.height > 0) {
                float sx = targetW / cs.width;
                float sy = targetH / cs.height;
                float s = std::min(sx, sy);
                _hotbarBgSprite->setScale(s);
            }
            useImageBg = true;
        }
    }
    if (!useImageBg) {
        auto bg = DrawNode::create();
        Vec2 bgVerts[4] = { Vec2(-totalWidth/2 - 10, -slotH/2 - 8),
                            Vec2( totalWidth/2 + 10, -slotH/2 - 8),
                            Vec2( totalWidth/2 + 10,  slotH/2 + 8),
                            Vec2(-totalWidth/2 - 10,  slotH/2 + 8) };
        bg->drawSolidPoly(bgVerts, 4, Color4F(0.f, 0.f, 0.f, 0.35f));
        _hotbarNode->addChild(bg);
    }

    _hotbarLabels.clear();
    _hotbarIcons.clear();
    for (int i = 0; i < slots; ++i) {
        float x = -totalWidth/2 + i * (slotW + padding) + slotW/2;
        if (!useImageBg) {
            auto rect = DrawNode::create();
            Vec2 r[4] = { Vec2(x - slotW/2, -slotH/2), Vec2(x + slotW/2, -slotH/2), Vec2(x + slotW/2,  slotH/2), Vec2(x - slotW/2,  slotH/2) };
            rect->drawSolidPoly(r, 4, Color4F(0.15f, 0.15f, 0.15f, 0.6f));
            _hotbarNode->addChild(rect);
        }
        auto icon = Sprite::create();
        icon->setPosition(Vec2(x, 0));
        icon->setVisible(false);
        _hotbarNode->addChild(icon, 1);
        _hotbarIcons.push_back(icon);
        std::string text = "-";
        if (_inventory) {
            if (auto t = _inventory->toolAt(i)) {
                text = t->displayName();
            } else if (_inventory->isItem(i)) {
                auto st = _inventory->itemAt(i);
                text = StringUtils::format("%s x%d", Game::itemName(st.type), st.quantity);
            }
        }
        auto label = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 18);
        label->setPosition(Vec2(x, 0));
        _hotbarNode->addChild(label, 2);
        _hotbarLabels.push_back(label);
    }

    _hotbarHighlight = DrawNode::create();
    _hotbarNode->addChild(_hotbarHighlight);
    {
        bool imageBg = (_hotbarBgSprite != nullptr);
        float cellHScaled = imageBg ? (_hotbarBgSprite->getContentSize().height * _hotbarBgSprite->getScaleY()) : slotH;
        float cellHBase = 32.0f;
        if (imageBg) {
            auto cs = _hotbarBgSprite->getContentSize();
            float slotWBase = 80.0f, slotHBase = 32.0f, paddingBase = 6.0f;
            float totalWidthBase = slots * slotWBase + (slots - 1) * paddingBase;
            float targetWBase = totalWidthBase + 20.0f;
            float targetHBase = slotHBase + 16.0f;
            float sxBase = targetWBase / cs.width;
            float syBase = targetHBase / cs.height;
            float sBase = std::min(sxBase, syBase);
            cellHBase = cs.height * sBase;
        }
        float baseY = origin.y + 28;
        float adjustY = (cellHScaled - cellHBase) * 0.5f;
        _hotbarNode->setPosition(Vec2(origin.x + visibleSize.width / 2, baseY + adjustY));
    }
    refreshHotbar();
}

void HotbarUI::refreshHotbar() {
    if (!_hotbarNode || !_hotbarHighlight || !_inventory) return;
    int slots = static_cast<int>(_inventory->size());
    if (slots <= 0) return;
    float slotW = 80.0f * _hotbarScale, slotH = 32.0f * _hotbarScale, padding = 6.0f * _hotbarScale;
    float totalWidth = slots * slotW + (slots - 1) * padding;
    bool imageBg = (_hotbarBgSprite != nullptr);
    float bgScaledW = 0.0f, bgScaledH = 0.0f;
    if (imageBg) {
        auto cs = _hotbarBgSprite->getContentSize();
        bgScaledW = cs.width * _hotbarBgSprite->getScaleX();
        bgScaledH = cs.height * _hotbarBgSprite->getScaleY();
    }
    float cellW = imageBg ? (bgScaledW / std::max(1, slots)) : slotW;
    float cellH = imageBg ? bgScaledH : slotH;
    int sel = _inventory->selectedIndex();
    float x = imageBg
        ? (-bgScaledW/2 + (sel + 0.5f) * cellW)
        : (-totalWidth/2 + sel * (slotW + padding) + slotW/2);
    _hotbarHighlight->clear();
    Vec2 a(x - cellW/2, -cellH/2), b(x + cellW/2, -cellH/2), c(x + cellW/2,  cellH/2), d(x - cellW/2,  cellH/2);
    _hotbarHighlight->drawLine(a, b, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _hotbarHighlight->drawLine(b, c, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _hotbarHighlight->drawLine(c, d, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _hotbarHighlight->drawLine(d, a, Color4F(1.f, 0.9f, 0.2f, 1.f));
    for (int i = 0; i < slots && i < static_cast<int>(_hotbarLabels.size()); ++i) {
        auto label = _hotbarLabels[i];
        auto icon = (i < static_cast<int>(_hotbarIcons.size())) ? _hotbarIcons[i] : nullptr;
        float cx = imageBg
            ? (-bgScaledW/2 + (i + 0.5f) * cellW)
            : (-totalWidth/2 + i * (slotW + padding) + slotW/2);

        if (auto tConst = _inventory->toolAt(i)) {
            auto t = _inventory->toolAtMutable(i);
            if (label) label->setVisible(false);
            if (icon) {
                std::string path;
                switch (tConst->kind()) {
                    case Game::ToolKind::Axe:        path = "Tool/Axe.png"; break;
                    case Game::ToolKind::Hoe:        path = "Tool/Hoe.png"; break;
                    case Game::ToolKind::Pickaxe:    path = "Tool/Pickaxe.png"; break;
                    case Game::ToolKind::WaterCan:   path = "Tool/WaterCan.png"; break;
                    case Game::ToolKind::FishingRod: path = "Tool/FishingRod.png"; break;
                    case Game::ToolKind::Sword:      path = "Weapon/sword.png"; break;
                    default: path.clear(); break;
                }
                if (!path.empty()) {
                    icon->setTexture(path);
                    if (icon->getTexture()) {
                        auto cs = icon->getContentSize();
                        float targetH = cellH;
                        float targetW = cellW;
                        float sx = (cs.width > 0) ? (targetW / cs.width) : 1.0f;
                        float sy = (cs.height > 0) ? (targetH / cs.height) : 1.0f;
                        float scale = std::min(sx, sy);
                        icon->setScale(scale);
                        icon->setPosition(Vec2(cx, 0));
                        icon->setVisible(true);
                        if (t) t->attachHotbarOverlay(icon, cellW, cellH);
                    } else {
                        icon->setVisible(false);
                        if (t) t->detachHotbarOverlay();
                        if (label) { // 回退显示工具名称
                            label->setString(tConst->displayName());
                            label->setPosition(Vec2(cx, 0));
                            label->setVisible(true);
                        }
                    }
                } else {
                    icon->setVisible(false);
                    if (t) t->detachHotbarOverlay();
                    if (label) { // 无图标路径时回退显示工具名称
                        label->setString(tConst->displayName());
                        label->setPosition(Vec2(cx, 0));
                        label->setVisible(true);
                    }
                }
            }
        } else if (_inventory->isItem(i)) {
            auto st = _inventory->itemAt(i);
            std::string text = StringUtils::format("%s x%d", Game::itemName(st.type), st.quantity);
            if (label) {
                label->setString(text);
                label->setPosition(Vec2(cx, 0));
                label->setVisible(true);
                if (st.type == Game::ItemType::Fish) {
                    label->setColor(Color3B(64, 200, 255));
                } else {
                    label->setColor(Color3B::WHITE);
                }
            }
            if (icon) {
                if (st.quantity > 0) {
                    std::string path;
                    if (st.type == Game::ItemType::Fish) {
                        path = "fish/globefish.png";
                    } else {
                        switch (st.type) {
                            case Game::ItemType::Coal:         path = "Mineral/Coal.png"; break;
                            case Game::ItemType::CopperGrain: path = "Mineral/copperGrain.png"; break;
                            case Game::ItemType::CopperIngot: path = "Mineral/copperIngot.png"; break;
                            case Game::ItemType::IronGrain:   path = "Mineral/ironGrain.png"; break;
                            case Game::ItemType::IronIngot:   path = "Mineral/ironIngot.png"; break;
                            case Game::ItemType::GoldGrain:   path = "Mineral/goldGrain.png"; break;
                            case Game::ItemType::GoldIngot:   path = "Mineral/goldIngot.png"; break;
                            default: break;
                        }
                    }
                    if (!path.empty()) {
                        icon->setTexture(path);
                        if (icon->getTexture()) {
                            auto cs = icon->getContentSize();
                            float targetH = cellH;
                            float targetW = cellW;
                            float sx = (cs.width > 0) ? (targetW / cs.width) : 1.0f;
                            float sy = (cs.height > 0) ? (targetH / cs.height) : 1.0f;
                            float scale = std::min(sx, sy);
                            icon->setScale(scale);
                            icon->setPosition(Vec2(cx, 0));
                            icon->setVisible(true);
                        } else {
                            icon->setVisible(false);
                        }
                    } else {
                        icon->setVisible(false);
                    }
                } else {
                    icon->setVisible(false);
                }
            }
        } else {
            if (label) {
                label->setString("-");
                label->setPosition(Vec2(cx, 0));
                label->setVisible(true);
                label->setColor(Color3B::WHITE);
            }
            if (icon) icon->setVisible(false);
        }
    }
}

void HotbarUI::selectHotbarIndex(int idx) {
    if (_inventory) {
        _inventory->selectIndex(idx);
        Game::globalState().selectedIndex = idx;
        refreshHotbar();
    }
}

bool HotbarUI::handleHotbarMouseDown(EventMouse* e) {
    if (!_hotbarNode || !_inventory) return false;
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return false;
    auto p = e->getLocation();
    auto local = _hotbarNode->convertToNodeSpace(p);
    int slots = static_cast<int>(_inventory->size());
    if (slots <= 0) return false;
    float slotW = 80.0f * _hotbarScale, slotH = 32.0f * _hotbarScale, padding = 6.0f * _hotbarScale, hitMarginY = 8.0f * _hotbarScale;
    bool imageBg = (_hotbarBgSprite != nullptr);
    float bgScaledW = 0.0f, bgScaledH = 0.0f;
    if (imageBg) {
        auto cs = _hotbarBgSprite->getContentSize();
        bgScaledW = cs.width * _hotbarBgSprite->getScaleX();
        bgScaledH = cs.height * _hotbarBgSprite->getScaleY();
    }
    float cellW = imageBg ? (bgScaledW / std::max(1, slots)) : slotW;
    float cellH = imageBg ? bgScaledH : slotH;
    float totalWidth = slots * slotW + (slots - 1) * padding;
    if (local.y < -(cellH/2 + hitMarginY) || local.y > (cellH/2 + hitMarginY)) return false;
    for (int i = 0; i < slots; ++i) {
        float cx = imageBg
            ? (-bgScaledW/2 + (i + 0.5f) * cellW)
            : (-totalWidth/2 + i * (slotW + padding) + slotW/2);
        float minx = cx - cellW/2;
        float maxx = cx + cellW/2;
        if (local.x >= minx && local.x <= maxx) {
            selectHotbarIndex(i);
            return true;
        }
    }
    return false;
}

bool HotbarUI::handleHotbarAtPoint(const Vec2& screenPoint) {
    if (!_hotbarNode || !_inventory) return false;
    auto local = _hotbarNode->convertToNodeSpace(screenPoint);
    int slots = static_cast<int>(_inventory->size());
    if (slots <= 0) return false;
    float slotW = 80.0f * _hotbarScale, slotH = 32.0f * _hotbarScale, padding = 6.0f * _hotbarScale, hitMarginY = 8.0f * _hotbarScale;
    bool imageBg = (_hotbarBgSprite != nullptr);
    float bgScaledW = 0.0f, bgScaledH = 0.0f;
    if (imageBg) {
        auto cs = _hotbarBgSprite->getContentSize();
        bgScaledW = cs.width * _hotbarBgSprite->getScaleX();
        bgScaledH = cs.height * _hotbarBgSprite->getScaleY();
    }
    float cellW = imageBg ? (bgScaledW / std::max(1, slots)) : slotW;
    float cellH = imageBg ? bgScaledH : slotH;
    float totalWidth = slots * slotW + (slots - 1) * padding;
    if (local.y < -(cellH/2 + hitMarginY) || local.y > (cellH/2 + hitMarginY)) return false;
    for (int i = 0; i < slots; ++i) {
        float cx = imageBg
            ? (-bgScaledW/2 + (i + 0.5f) * cellW)
            : (-totalWidth/2 + i * (slotW + padding) + slotW/2);
        float minx = cx - cellW/2;
        float maxx = cx + cellW/2;
        if (local.x >= minx && local.x <= maxx) {
            selectHotbarIndex(i);
            return true;
        }
    }
    return false;
}

void HotbarUI::handleHotbarScroll(float dy) {
    if (!_inventory) return;
    if (dy > 0) {
        _inventory->next();
    } else if (dy < 0) {
        _inventory->prev();
    } else {
        return;
    }
    Game::globalState().selectedIndex = _inventory->selectedIndex();
    refreshHotbar();
}

}
