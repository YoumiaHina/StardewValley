#include "Controllers/UI/ChestPanelUI.h"
#include "Game/WorldState.h"
#include "Game/Item.h"
#include "Game/Chest.h"

using namespace cocos2d;

namespace Controllers {

void ChestPanelUI::buildChestPanel() {
    if (_panelNode) return;
    _panelNode = Node::create();
    if (_scene) {
        _scene->addChild(_panelNode, 5);
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();
        _panelNode->setPosition(Vec2(origin.x + visibleSize.width * 0.5f,
                                     origin.y + visibleSize.height * 0.5f));
        _panelNode->setVisible(false);
    }
}

void ChestPanelUI::refreshChestPanel(Game::Chest* chest) {
    if (!chest) return;
    _currentChest = chest;
    if (!_panelNode) buildChestPanel();
    if (!_panelNode) return;
    _panelNode->removeAllChildren();
    _slotsRoot = nullptr;
    _highlightNode = nullptr;
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float panelW = visibleSize.width * 0.8f;
    float panelH = visibleSize.height * 0.5f;
    int cols = 12;
    int rows = 3;
    float cellW = 40.f;
    float cellH = 40.f;
    float startX = 0.f;
    float firstRowY = 0.f;
    float rowGap = cellH;
    auto templateSprite = Sprite::create("inventory.png");
    bool useTemplate = false;
    if (templateSprite && templateSprite->getTexture()) {
        auto cs = templateSprite->getContentSize();
        if (cs.width > 0 && cs.height > 0) {
            float sx = panelW / cs.width;
            float sy = panelH / (cs.height * 3.0f);
            float s = std::min(sx, sy);
            float pieceH = cs.height * s;
            float totalH = pieceH * 3.0f;
            auto bgRoot = Node::create();
            for (int i = 0; i < 3; ++i) {
                auto bg = Sprite::create("inventory.png");
                if (!bg || !bg->getTexture()) continue;
                bg->setScale(s);
                float y = -totalH * 0.5f + pieceH * 0.5f + i * pieceH;
                bg->setPosition(Vec2(0, y));
                bgRoot->addChild(bg);
            }
            _panelNode->addChild(bgRoot);
            float scaledW = cs.width * s;
            cellW = scaledW / static_cast<float>(cols);
            cellH = pieceH;
            startX = -scaledW * 0.5f + cellW * 0.5f;
            firstRowY = -totalH * 0.5f + pieceH * 0.5f;
            rowGap = pieceH;
            useTemplate = true;
        }
    }
    if (!useTemplate) {
        auto fallback = DrawNode::create();
        Vec2 v[4] = { Vec2(-panelW/2, -panelH/2), Vec2(panelW/2, -panelH/2),
                      Vec2(panelW/2, panelH/2), Vec2(-panelW/2, panelH/2) };
        fallback->drawSolidPoly(v, 4, Color4F(0.f,0.f,0.f,0.85f));
        _panelNode->addChild(fallback);
        float totalW = cols * cellW;
        float totalH = rows * cellH;
        startX = -totalW * 0.5f + cellW * 0.5f;
        firstRowY = totalH * 0.5f - cellH * 0.5f - 16.f;
        rowGap = cellH;
    }
    auto title = Label::createWithTTF("Chest", "fonts/arial.ttf", 22);
    title->setPosition(Vec2(0, panelH * 0.5f - 26));
    _panelNode->addChild(title);
    auto closeLabel = Label::createWithTTF("X", "fonts/arial.ttf", 26);
    closeLabel->setPosition(Vec2(panelW * 0.5f - 24.f, panelH * 0.5f - 24.f));
    _panelNode->addChild(closeLabel);
    auto closeListener = EventListenerTouchOneByOne::create();
    closeListener->setSwallowTouches(true);
    closeListener->onTouchBegan = [this, panelW, panelH](Touch* t, Event*){
        Vec2 p = _panelNode->convertToNodeSpace(t->getLocation());
        float regionSize = 64.f;
        Rect r(panelW * 0.5f - regionSize, panelH * 0.5f - regionSize, regionSize, regionSize);
        if (r.containsPoint(p)) {
            return true;
        }
        return false;
    };
    closeListener->onTouchEnded = [this, panelW, panelH](Touch* t, Event*){
        Vec2 p = _panelNode->convertToNodeSpace(t->getLocation());
        float regionSize = 64.f;
        Rect r(panelW * 0.5f - regionSize, panelH * 0.5f - regionSize, regionSize, regionSize);
        if (r.containsPoint(p)) {
            toggleChestPanel(false);
        }
    };
    _panelNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(closeListener, _panelNode);
    _slotsRoot = Node::create();
    _panelNode->addChild(_slotsRoot, 2);
    _highlightNode = DrawNode::create();
    _slotsRoot->addChild(_highlightNode, 11);
    auto allSlotsDebug = DrawNode::create();
    _slotsRoot->addChild(allSlotsDebug, 10);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            float cx = startX + c * cellW;
            float cy = firstRowY - r * rowGap;
            cocos2d::Rect cellRect(cx - cellW * 0.5f, cy - cellH * 0.5f, cellW, cellH);
            auto cellNode = Node::create();
            cellNode->setPosition(Vec2(cx, cy));
            _slotsRoot->addChild(cellNode);
            int flatIndex = r * cols + c;
            auto icon = Sprite::create();
            icon->setPosition(Vec2(0, 0));
            icon->setVisible(false);
            cellNode->addChild(icon);
            auto countLabel = Label::createWithTTF("", "fonts/arial.ttf", 14);
            countLabel->setAnchorPoint(Vec2(1.f, 0.f));
            countLabel->setPosition(Vec2(cellW * 0.5f - 6.f, -cellH * 0.5f + 4.f));
            countLabel->setVisible(false);
            cellNode->addChild(countLabel);
            Game::Chest* chestPtr = chest;
            auto updateCell = [icon, countLabel, chestPtr, flatIndex, cellW, cellH]() {
                if (!chestPtr || flatIndex < 0 || flatIndex >= static_cast<int>(chestPtr->slots.size())) {
                    icon->setVisible(false);
                    countLabel->setVisible(false);
                    return;
                }
                const auto& slot = chestPtr->slots[static_cast<std::size_t>(flatIndex)];
                if (slot.kind == Game::SlotKind::Item && slot.itemQty > 0) {
                    std::string path = Game::itemIconPath(slot.itemType);
                    if (path.empty()) {
                        switch (slot.itemType) {
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
                    }
                    if (icon->getTexture()) {
                        auto cs = icon->getContentSize();
                        float targetW = cellW - 8.f;
                        float targetH = cellH - 8.f;
                        float sx = cs.width > 0 ? targetW / cs.width : 1.0f;
                        float sy = cs.height > 0 ? targetH / cs.height : 1.0f;
                        float s = std::min(sx, sy);
                        icon->setScale(s);
                        icon->setVisible(true);
                    } else {
                        icon->setVisible(false);
                    }
                    countLabel->setString(StringUtils::format("%d", slot.itemQty));
                    countLabel->setVisible(true);
                } else {
                    icon->setVisible(false);
                    countLabel->setVisible(false);
                }
            };
            updateCell();
            float offsetY = cellH * 2.0f;
            float hwDebug = cellW * 0.5f;
            float hhDebug = cellH * 0.5f;
            float cyDebug = cy + offsetY;
            Vec2 da(cx - hwDebug, cyDebug - hhDebug);
            Vec2 db(cx + hwDebug, cyDebug - hhDebug);
            Vec2 dc(cx + hwDebug, cyDebug + hhDebug);
            Vec2 dd(cx - hwDebug, cyDebug + hhDebug);
            Color4F dbgColor(1.f, 1.f, 1.f, 1.f);
            allSlotsDebug->drawLine(da, db, dbgColor);
            allSlotsDebug->drawLine(db, dc, dbgColor);
            allSlotsDebug->drawLine(dc, dd, dbgColor);
            allSlotsDebug->drawLine(dd, da, dbgColor);
        }
    }
    auto slotsListener = EventListenerTouchOneByOne::create();
    slotsListener->setSwallowTouches(false);
    slotsListener->onTouchBegan = [this, cols, rows, cellW, cellH, startX, firstRowY, rowGap](Touch* t, Event*){
        if (!_slotsRoot) return false;
        float offsetY = cellH * 2.0f;
        Vec2 p = _slotsRoot->convertToNodeSpace(t->getLocation());
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                float cx = startX + c * cellW;
                float cy = firstRowY - r * rowGap;
                float cyHit = cy + offsetY;
                Rect hitRect(cx - cellW * 0.5f, cyHit - cellH * 0.5f, cellW, cellH);
                if (hitRect.containsPoint(p)) {
                    return true;
                }
            }
        }
        return false;
    };
    slotsListener->onTouchEnded = [this, cols, rows, cellW, cellH, startX, firstRowY, rowGap](Touch* t, Event*){
        if (!_currentChest || !_inventory || !_slotsRoot) return;
        float offsetY = cellH * 2.0f;
        Vec2 p = _slotsRoot->convertToNodeSpace(t->getLocation());
        int hitIndex = -1;
        float hitCx = 0.f;
        float hitCyHit = 0.f;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                float cx = startX + c * cellW;
                float cy = firstRowY - r * rowGap;
                float cyHit = cy + offsetY;
                Rect hitRect(cx - cellW * 0.5f, cyHit - cellH * 0.5f, cellW, cellH);
                if (hitRect.containsPoint(p)) {
                    hitIndex = r * cols + c;
                    hitCx = cx;
                    hitCyHit = cyHit;
                    break;
                }
            }
            if (hitIndex >= 0) break;
        }
        if (hitIndex < 0) return;
        if (!_highlightNode && _slotsRoot) {
            _highlightNode = DrawNode::create();
            _slotsRoot->addChild(_highlightNode, 10);
        }
        if (!_highlightNode) return;
        _highlightNode->clear();
        float hw = cellW * 0.5f;
        float hh = cellH * 0.5f;
        Vec2 a(hitCx - hw, hitCyHit - hh);
        Vec2 b(hitCx + hw, hitCyHit - hh);
        Vec2 c(hitCx + hw, hitCyHit + hh);
        Vec2 d(hitCx - hw, hitCyHit + hh);
        Color4F color(0.f, 0.f, 0.f, 1.f);
        _highlightNode->drawLine(a, b, color);
        _highlightNode->drawLine(b, c, color);
        _highlightNode->drawLine(c, d, color);
        _highlightNode->drawLine(d, a, color);
    };
    _panelNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(slotsListener, _slotsRoot);
}

void ChestPanelUI::toggleChestPanel(bool show) {
    if (_panelNode) _panelNode->setVisible(show);
}

bool ChestPanelUI::isVisible() const {
    return _panelNode && _panelNode->isVisible();
}

}
