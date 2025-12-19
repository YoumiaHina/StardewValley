#include "Controllers/UI/ChestPanelUI.h"
#include "Controllers/Systems/ChestController.h"
#include "Game/WorldState.h"
#include "Game/Item.h"
#include "Game/Chest.h"
#include "Game/Tool/ToolBase.h"
#include "Game/Tool/ToolFactory.h"

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
        if (!_escListener) {
            _escListener = EventListenerKeyboard::create();
            _escListener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*) {
                if (code == EventKeyboard::KeyCode::KEY_SHIFT ||
                    code == EventKeyboard::KeyCode::KEY_LEFT_SHIFT ||
                    code == EventKeyboard::KeyCode::KEY_RIGHT_SHIFT) {
                    _shiftDown = true;
                }
            };
            _escListener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) {
                if (code == EventKeyboard::KeyCode::KEY_ESCAPE) {
                    toggleChestPanel(false);
                }
                if (code == EventKeyboard::KeyCode::KEY_SHIFT ||
                    code == EventKeyboard::KeyCode::KEY_LEFT_SHIFT ||
                    code == EventKeyboard::KeyCode::KEY_RIGHT_SHIFT) {
                    _shiftDown = false;
                }
            };
            _panelNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_escListener, _panelNode);
        }
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
    _cellIcons.clear();
    _cellCountLabels.clear();
    _cellNameLabels.clear();
    _selectedIndex = -1;
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
    auto closeLabel = Label::createWithTTF("X", "fonts/arial.ttf", 26);
    closeLabel->setPosition(Vec2(panelW * 0.5f - 16.f, panelH * 0.5f - 16.f));
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
    int capacity = rows * cols;
    _cellIcons.assign(capacity, nullptr);
    _cellCountLabels.assign(capacity, nullptr);
    _cellNameLabels.assign(capacity, nullptr);
    float offsetYVisual = cellH * 2.0f;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            float cx = startX + c * cellW;
            float cy = firstRowY - r * rowGap;
            cocos2d::Rect cellRect(cx - cellW * 0.5f, cy - cellH * 0.5f, cellW, cellH);
            auto cellNode = Node::create();
            cellNode->setPosition(Vec2(cx, cy + offsetYVisual));
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
            auto nameLabel = Label::createWithTTF("", "fonts/arial.ttf", 14);
            nameLabel->setAnchorPoint(Vec2(0.5f, 0.f));
            nameLabel->setPosition(Vec2(0.f, -cellH * 0.5f + 20.f));
            nameLabel->setVisible(false);
            cellNode->addChild(nameLabel);
            if (flatIndex >= 0 && flatIndex < capacity) {
                _cellIcons[flatIndex] = icon;
                _cellCountLabels[flatIndex] = countLabel;
                _cellNameLabels[flatIndex] = nameLabel;
            }
            Game::Chest* chestPtr = chest;
            auto updateCell = [icon, countLabel, nameLabel, chestPtr, flatIndex, cellW, cellH]() {
                if (!chestPtr || flatIndex < 0 || flatIndex >= static_cast<int>(chestPtr->slots.size())) {
                    icon->setVisible(false);
                    countLabel->setVisible(false);
                    nameLabel->setVisible(false);
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
                    std::string name = Game::itemName(slot.itemType);
                    nameLabel->setString(name);
                    nameLabel->setVisible(true);
                } else if (slot.kind == Game::SlotKind::Tool && slot.tool) {
                    std::string path;
                    auto t = slot.tool.get();
                    if (t) {
                        switch (t->kind()) {
                            case Game::ToolKind::Axe:        path = "Tool/Axe.png"; break;
                            case Game::ToolKind::Hoe:        path = "Tool/Hoe.png"; break;
                            case Game::ToolKind::Pickaxe:    path = "Tool/Pickaxe.png"; break;
                            case Game::ToolKind::WaterCan:   path = "Tool/WaterCan.png"; break;
                            case Game::ToolKind::FishingRod: path = "Tool/FishingRod.png"; break;
                            case Game::ToolKind::Sword:      path = "Weapon/sword.png"; break;
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
                    countLabel->setVisible(false);
                    std::string name = t ? t->displayName() : std::string();
                    if (!name.empty()) {
                        nameLabel->setString(name);
                        nameLabel->setVisible(true);
                    } else {
                        nameLabel->setVisible(false);
                    }
                } else {
                    icon->setVisible(false);
                    countLabel->setVisible(false);
                    nameLabel->setVisible(false);
                }
            };
            updateCell();
        }
    }
    auto slotsListener = EventListenerTouchOneByOne::create();
    slotsListener->setSwallowTouches(false);
    slotsListener->onTouchBegan = [this, cols, rows, cellW, cellH, startX, firstRowY, rowGap](Touch* t, Event*){
        if (!_slotsRoot || !_panelNode || !_panelNode->isVisible()) return false;
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
        if (!_currentChest || !_inventory || !_slotsRoot || !_panelNode || !_panelNode->isVisible()) return;
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
        _selectedIndex = hitIndex;
        if (_shiftDown) {
            int invIndex = _inventory->selectedIndex();
            if (invIndex >= 0 && invIndex < static_cast<int>(_inventory->size())) {
                int maxMoves = Game::ItemStack::MAX_STACK;
                for (int i = 0; i < maxMoves; ++i) {
                    if (hitIndex < 0 || hitIndex >= static_cast<int>(_currentChest->slots.size())) {
                        break;
                    }
                    Game::Slot beforeChest = _currentChest->slots[static_cast<std::size_t>(hitIndex)];
                    Game::ItemStack beforeInvStack = _inventory->itemAt(static_cast<std::size_t>(invIndex));
                    bool beforeInvEmpty = _inventory->isEmpty(static_cast<std::size_t>(invIndex));
                    bool beforeInvIsTool = _inventory->isTool(static_cast<std::size_t>(invIndex));
                    transferChestCell(*_currentChest, hitIndex, *_inventory);
                    if (hitIndex < 0 || hitIndex >= static_cast<int>(_currentChest->slots.size())) {
                        break;
                    }
                    const Game::Slot& afterChest = _currentChest->slots[static_cast<std::size_t>(hitIndex)];
                    Game::ItemStack afterInvStack = _inventory->itemAt(static_cast<std::size_t>(invIndex));
                    bool afterInvEmpty = _inventory->isEmpty(static_cast<std::size_t>(invIndex));
                    bool afterInvIsTool = _inventory->isTool(static_cast<std::size_t>(invIndex));
                    bool chestSame = (beforeChest.kind == afterChest.kind &&
                                      beforeChest.itemType == afterChest.itemType &&
                                      beforeChest.itemQty == afterChest.itemQty);
                    bool invSame = (beforeInvEmpty == afterInvEmpty &&
                                    beforeInvIsTool == afterInvIsTool &&
                                    beforeInvStack.type == afterInvStack.type &&
                                    beforeInvStack.quantity == afterInvStack.quantity);
                    if (chestSame && invSame) {
                        break;
                    }
                }
            }
        } else {
            transferChestCell(*_currentChest, hitIndex, *_inventory);
        }
        if (hitIndex >= 0 && hitIndex < static_cast<int>(_cellIcons.size())) {
            auto icon = _cellIcons[hitIndex];
            auto countLabel = _cellCountLabels[hitIndex];
            auto nameLabel = _cellNameLabels[hitIndex];
            if (icon && countLabel && nameLabel) {
                const auto& slot = _currentChest->slots[static_cast<std::size_t>(hitIndex)];
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
                    std::string name = Game::itemName(slot.itemType);
                    nameLabel->setString(name);
                    nameLabel->setVisible(true);
                } else if (slot.kind == Game::SlotKind::Tool && slot.tool) {
                    std::string path;
                    auto t = slot.tool.get();
                    if (t) {
                        switch (t->kind()) {
                            case Game::ToolKind::Axe:        path = "Tool/Axe.png"; break;
                            case Game::ToolKind::Hoe:        path = "Tool/Hoe.png"; break;
                            case Game::ToolKind::Pickaxe:    path = "Tool/Pickaxe.png"; break;
                            case Game::ToolKind::WaterCan:   path = "Tool/WaterCan.png"; break;
                            case Game::ToolKind::FishingRod: path = "Tool/FishingRod.png"; break;
                            case Game::ToolKind::Sword:      path = "Weapon/sword.png"; break;
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
                    countLabel->setVisible(false);
                    std::string name = t ? t->displayName() : std::string();
                    if (!name.empty()) {
                        nameLabel->setString(name);
                        nameLabel->setVisible(true);
                    } else {
                        nameLabel->setVisible(false);
                    }
                } else {
                    icon->setVisible(false);
                    countLabel->setVisible(false);
                    nameLabel->setVisible(false);
                }
            }
        }
        if (_onInventoryChanged) {
            _onInventoryChanged();
        }
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

void ChestPanelUI::setOnInventoryChanged(const std::function<void()>& cb) {
    _onInventoryChanged = cb;
}

void ChestPanelUI::onInventorySlotClicked(int invIndex) {
    if (!_currentChest || !_inventory) return;
    if (_selectedIndex < 0) return;
    if (invIndex < 0 || invIndex >= static_cast<int>(_inventory->size())) return;
    if (_selectedIndex < 0 || _selectedIndex >= static_cast<int>(_currentChest->slots.size())) return;
    auto& slotChest = _currentChest->slots[static_cast<std::size_t>(_selectedIndex)];
    if (slotChest.kind == Game::SlotKind::Item && slotChest.itemQty > 0) {
        Game::ItemType type = slotChest.itemType;
        bool invEmpty = _inventory->isEmpty(static_cast<std::size_t>(invIndex));
        bool invIsItem = _inventory->isItem(static_cast<std::size_t>(invIndex));
        Game::ItemStack st = _inventory->itemAt(static_cast<std::size_t>(invIndex));
        bool sameType = invIsItem && st.type == type;
        int currentQty = (invEmpty || !invIsItem) ? 0 : st.quantity;
        int space = Game::ItemStack::MAX_STACK - currentQty;
        bool canReceive = (invEmpty || sameType) && (space > 0);
        if (!canReceive) return;
        int moveCount = _shiftDown ? std::min(space, slotChest.itemQty) : 1;
        for (int i = 0; i < moveCount; ++i) {
            bool okAdd = _inventory->addOneItemToSlot(static_cast<std::size_t>(invIndex), type);
            if (!okAdd) break;
            if (slotChest.itemQty <= 0) break;
            slotChest.itemQty -= 1;
            if (slotChest.itemQty <= 0) {
                slotChest.kind = Game::SlotKind::Empty;
                slotChest.itemQty = 0;
                break;
            }
        }
    } else if (slotChest.kind == Game::SlotKind::Tool && slotChest.tool) {
        bool invEmpty = _inventory->isEmpty(static_cast<std::size_t>(invIndex));
        if (!invEmpty) return;
        auto t = slotChest.tool.get();
        if (!t) return;
        Game::ToolKind tk = t->kind();
        _inventory->setTool(static_cast<std::size_t>(invIndex), Game::makeTool(tk));
        slotChest.tool.reset();
        slotChest.kind = Game::SlotKind::Empty;
        slotChest.itemQty = 0;
    } else {
        return;
    }
    if (_selectedIndex >= 0 && _selectedIndex < static_cast<int>(_cellIcons.size())) {
        auto icon = _cellIcons[_selectedIndex];
        auto countLabel = _cellCountLabels[_selectedIndex];
        auto nameLabel = _cellNameLabels[_selectedIndex];
        if (icon && countLabel && nameLabel) {
            const auto& slot = _currentChest->slots[static_cast<std::size_t>(_selectedIndex)];
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
                    icon->setVisible(true);
                } else {
                    icon->setVisible(false);
                }
                countLabel->setString(StringUtils::format("%d", slot.itemQty));
                countLabel->setVisible(true);
                std::string name = Game::itemName(slot.itemType);
                nameLabel->setString(name);
                nameLabel->setVisible(true);
            } else if (slot.kind == Game::SlotKind::Tool && slot.tool) {
                std::string path;
                auto t = slot.tool.get();
                if (t) {
                    switch (t->kind()) {
                        case Game::ToolKind::Axe:        path = "Tool/Axe.png"; break;
                        case Game::ToolKind::Hoe:        path = "Tool/Hoe.png"; break;
                        case Game::ToolKind::Pickaxe:    path = "Tool/Pickaxe.png"; break;
                        case Game::ToolKind::WaterCan:   path = "Tool/WaterCan.png"; break;
                        case Game::ToolKind::FishingRod: path = "Tool/FishingRod.png"; break;
                        case Game::ToolKind::Sword:      path = "Weapon/sword.png"; break;
                        default: break;
                    }
                }
                if (!path.empty()) {
                    icon->setTexture(path);
                }
                if (icon->getTexture()) {
                    icon->setVisible(true);
                } else {
                    icon->setVisible(false);
                }
                countLabel->setVisible(false);
                std::string name = t ? t->displayName() : std::string();
                if (!name.empty()) {
                    nameLabel->setString(name);
                    nameLabel->setVisible(true);
                } else {
                    nameLabel->setVisible(false);
                }
            } else {
                icon->setVisible(false);
                countLabel->setVisible(false);
                nameLabel->setVisible(false);
            }
        }
    }
    if (_onInventoryChanged) {
        _onInventoryChanged();
    }
}

void ChestPanelUI::toggleChestPanel(bool show) {
    if (_panelNode) _panelNode->setVisible(show);
}

bool ChestPanelUI::isVisible() const {
    return _panelNode && _panelNode->isVisible();
}

}
