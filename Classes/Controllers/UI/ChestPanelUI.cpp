#include "Controllers/UI/ChestPanelUI.h"
#include "Controllers/Systems/ChestController.h"
#include "Game/WorldState.h"
#include "Game/Item.h"
#include "Game/Chest.h"
#include "Game/Tool/ToolBase.h"
#include "Game/Tool/ToolFactory.h"

using namespace cocos2d;

namespace Controllers {

// 构建箱子面板根节点：挂载到场景，并注册 ESC/Shift 键盘监听。
// - 只会在第一次调用时创建 Node 和键盘监听，后续复用同一套节点。
// - 使用 Director::getInstance()->getVisibleSize/Origin 获取当前窗口大小与原点。
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
        // 仅在第一次构建时创建键盘监听器。
        if (!_escListener) {
            // EventListenerKeyboard：专门用于处理键盘按下/抬起事件的监听器。
            _escListener = EventListenerKeyboard::create();
            // onKeyPressed 使用 lambda 捕获 this，方便修改成员变量 _shiftDown。
            _escListener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*) {
                if (code == EventKeyboard::KeyCode::KEY_SHIFT ||
                    code == EventKeyboard::KeyCode::KEY_LEFT_SHIFT ||
                    code == EventKeyboard::KeyCode::KEY_RIGHT_SHIFT) {
                    _shiftDown = true;
                }
            };
            // onKeyReleased：当 ESC 被抬起时关闭箱子面板；Shift 抬起时复位 _shiftDown。
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

// 使用指定 Chest 重建箱子 UI：包括背景、格子布局与初始图标。
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
    auto templateSprite = Sprite::create("inventory1.png");
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
                auto bg = Sprite::create("inventory1.png");
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
    // 右上角关闭按钮（简单用一个 “X” 文字代替）。
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
    // 用于承载所有格子节点的根节点。
    _slotsRoot = Node::create();
    _panelNode->addChild(_slotsRoot, 2);
    _highlightNode = DrawNode::create();
    _slotsRoot->addChild(_highlightNode, 11);
    // 箱子容量：行数 * 列数。
    int capacity = rows * cols;
    _cellIcons.assign(capacity, nullptr);
    _cellCountLabels.assign(capacity, nullptr);
    _cellNameLabels.assign(capacity, nullptr);
    float offsetYVisual = cellH * 2.0f;
    // 为每个格子创建子节点，内部再挂载图标与数量/名称文本。
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
            auto countLabel = Label::createWithTTF("", "fonts/arial.ttf", 18);
            countLabel->setAnchorPoint(Vec2(1.f, 0.f));
            countLabel->setColor(Color3B::BLACK);
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
            // 为当前格子构造一个“小更新函数”：根据 Chest 槽位刷新图标与数量。
            Game::Chest* chestPtr = chest;
            auto updateCell = [icon, countLabel, nameLabel, chestPtr, flatIndex, cellW, cellH]() {
                if (!chestPtr || flatIndex < 0 || flatIndex >= static_cast<int>(chestPtr->slots.size())) {
                    icon->setVisible(false);
                    countLabel->setVisible(false);
                    nameLabel->setVisible(false);
                    return;
                }
                const auto& slot = chestPtr->slots[static_cast<std::size_t>(flatIndex)];
                nameLabel->setVisible(false);
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
                if (slot.itemQty > 1) {
                    countLabel->setString(StringUtils::format("%d", slot.itemQty));
                    countLabel->setVisible(true);
                } else {
                    countLabel->setVisible(false);
                }
                } else if (slot.kind == Game::SlotKind::Tool && slot.tool) {
                    auto t = slot.tool.get();
                    std::string path = t ? t->iconPath() : std::string();
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
                    nameLabel->setVisible(false);
                } else {
                    icon->setVisible(false);
                    countLabel->setVisible(false);
                    nameLabel->setVisible(false);
                }
            };
            updateCell();
        }
    }
    // 为所有格子注册点击监听：负责命中检测和触发背包->箱子转移。
    auto slotsListener = EventListenerTouchOneByOne::create();
    slotsListener->setSwallowTouches(false);
    // onTouchBegan：只做命中测试，判断是否触碰到任意格子。
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
    // onTouchEnded：定位被点击的格子索引，并调用 transferInventoryToChest 执行背包->箱子转移。
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
        transferInventoryToChest(*_currentChest, hitIndex, *_inventory, _shiftDown);
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
                    if (slot.itemQty > 1) {
                        countLabel->setString(StringUtils::format("%d", slot.itemQty));
                        float offsetX = cellW * 0.5f - 6.f;
                        float offsetY = -cellH * 0.5f + 4.f;
                        countLabel->setPosition(Vec2(offsetX, offsetY));
                        countLabel->setVisible(true);
                    } else {
                        countLabel->setVisible(false);
                    }
                    nameLabel->setVisible(false);
                } else if (slot.kind == Game::SlotKind::Tool && slot.tool) {
                    auto t = slot.tool.get();
                    std::string path = t ? t->iconPath() : std::string();
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
                    nameLabel->setVisible(false);
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
        Color4F color(1.f, 0.f, 0.f, 1.f);
        _highlightNode->drawLine(a, b, color);
        _highlightNode->drawLine(b, c, color);
        _highlightNode->drawLine(c, d, color);
        _highlightNode->drawLine(d, a, color);
    };
    _panelNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(slotsListener, _slotsRoot);
}

// 设置背包变更回调：箱子/背包之间发生转移时由面板触发。
void ChestPanelUI::setOnInventoryChanged(const std::function<void()>& cb) {
    _onInventoryChanged = cb;
}

// 当热键栏/背包格被点击时调用：尝试将当前选中的箱子格内容转移到指定背包格。
void ChestPanelUI::onInventorySlotClicked(int invIndex) {
    if (!_currentChest || !_inventory) return;
    if (_selectedIndex < 0) return;
    if (invIndex < 0 || invIndex >= static_cast<int>(_inventory->size())) return;
    if (_selectedIndex < 0 || _selectedIndex >= static_cast<int>(_currentChest->slots.size())) return;

    bool moved = transferChestToInventory(
        *_currentChest,
        _selectedIndex,
        *_inventory,
        invIndex,
        _shiftDown);
    if (!moved) return;

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
                if (slot.itemQty > 1) {
                    countLabel->setString(StringUtils::format("%d", slot.itemQty));
                    countLabel->setVisible(true);
                } else {
                    countLabel->setVisible(false);
                }
                nameLabel->setVisible(false);
            } else if (slot.kind == Game::SlotKind::Tool && slot.tool) {
                std::string path;
                auto t = slot.tool.get();
                if (t) {
                    path = t->iconPath();
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
                nameLabel->setVisible(false);
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

// 显示或隐藏箱子面板。
void ChestPanelUI::toggleChestPanel(bool show) {
    if (_panelNode) _panelNode->setVisible(show);
}

// 返回箱子面板当前是否可见。
bool ChestPanelUI::isVisible() const {
    return _panelNode && _panelNode->isVisible();
}

}
