#include "Controllers/UI/UIController.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"

using namespace cocos2d;

namespace Controllers {

void UIController::buildHUD() {
    auto &ws = Game::globalState();
    if (ws.maxEnergy <= 0) ws.maxEnergy = GameConfig::ENERGY_MAX;
    if (ws.energy < 0 || ws.energy > ws.maxEnergy) ws.energy = ws.maxEnergy;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    if (!_hudTimeLabel) {
        _hudTimeLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
        _hudTimeLabel->setColor(Color3B::WHITE);
        _hudTimeLabel->setAnchorPoint(Vec2(1,1));
        float pad = 10.0f;
        _hudTimeLabel->setPosition(Vec2(origin.x + visibleSize.width - pad, origin.y + visibleSize.height - pad));
        if (_scene) _scene->addChild(_hudTimeLabel, 3);
    }

    if (!_energyNode) {
        _energyNode = Node::create();
        float pad = 10.0f;
        _energyNode->setPosition(Vec2(origin.x + visibleSize.width - pad, origin.y + pad));
        if (_scene) _scene->addChild(_energyNode, 3);

        float bw = 160.0f, bh = 18.0f;
        auto bg = DrawNode::create();
        Vec2 bl(-bw, 0), br(0, 0), tr(0, bh), tl(-bw, bh);
        Vec2 rect[4] = { bl, br, tr, tl };
        bg->drawSolidPoly(rect, 4, Color4F(0.f,0.f,0.f,0.35f));
        bg->drawLine(bl, br, Color4F(1,1,1,0.5f));
        bg->drawLine(br, tr, Color4F(1,1,1,0.5f));
        bg->drawLine(tr, tl, Color4F(1,1,1,0.5f));
        bg->drawLine(tl, bl, Color4F(1,1,1,0.5f));
        _energyNode->addChild(bg);

        _energyFill = DrawNode::create();
        _energyNode->addChild(_energyFill);

        _energyLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 16);
        _energyLabel->setAnchorPoint(Vec2(1,0.5f));
        _energyLabel->setPosition(Vec2(-4.0f, bh * 0.5f));
        _energyLabel->setColor(Color3B::WHITE);
        _energyNode->addChild(_energyLabel);
    }

    refreshHUD();
}

void UIController::refreshHUD() {
    auto &ws = Game::globalState();
    auto seasonName = [](int idx){
        switch (idx % 4) { case 0: return "Spring"; case 1: return "Summer"; case 2: return "Fall"; default: return "Winter"; }
    };
    if (_hudTimeLabel) {
        _hudTimeLabel->setString(StringUtils::format("%s Day %d, %02d:%02d", seasonName(ws.seasonIndex), ws.dayOfSeason, ws.timeHour, ws.timeMinute));
    }
    if (_energyFill && _energyNode) {
        _energyFill->clear();
        float bw = 160.0f, bh = 18.0f;
        float ratio = ws.maxEnergy > 0 ? (static_cast<float>(ws.energy) / static_cast<float>(ws.maxEnergy)) : 0.f;
        ratio = std::max(0.f, std::min(1.f, ratio));
        float fillW = bw * ratio;
        Vec2 bl(-bw, 0), br(-bw + fillW, 0), tr(-bw + fillW, bh), tl(-bw, bh);
        Vec2 rect[4] = { bl, br, tr, tl };
        _energyFill->drawSolidPoly(rect, 4, Color4F(0.2f, 0.8f, 0.25f, 0.85f));
    }
    if (_energyLabel) {
        _energyLabel->setString(StringUtils::format("Energy %d/%d", ws.energy, ws.maxEnergy));
    }
    // HP HUD（矿洞专属）
    if (_hpFill && _hpNode) {
        _hpFill->clear();
        float bw = 160.0f, bh = 18.0f;
        float ratio = ws.maxHp > 0 ? (static_cast<float>(ws.hp) / static_cast<float>(ws.maxHp)) : 0.f;
        ratio = std::max(0.f, std::min(1.f, ratio));
        float fillW = bw * ratio;
        Vec2 bl(-bw, 0), br(-bw + fillW, 0), tr(-bw + fillW, bh), tl(-bw, bh);
        Vec2 rect[4] = { bl, br, tr, tl };
        _hpFill->drawSolidPoly(rect, 4, Color4F(0.9f, 0.15f, 0.15f, 0.95f));
    }
    if (_hpLabel) {
        _hpLabel->setString(StringUtils::format("HP %d/%d", ws.hp, ws.maxHp));
    }
}

void UIController::buildHPBarAboveEnergy() {
    // 仅当能量条已建立时创建 HP 条，并将其放置在能量条正上方
    if (!_energyNode || _hpNode) return;
    _hpNode = Node::create();
    // 根据能量条的世界位置放置到其正上方
    Vec2 energyWorld = _energyNode->getPosition();
    float offsetY = 24.0f; // 与能量条间距
    _hpNode->setPosition(Vec2(energyWorld.x, energyWorld.y + offsetY));
    if (_scene) _scene->addChild(_hpNode, 3);

    float bw = 160.0f, bh = 18.0f;
    auto bg = DrawNode::create();
    Vec2 bl(-bw, 0), br(0, 0), tr(0, bh), tl(-bw, bh);
    Vec2 rect[4] = { bl, br, tr, tl };
    bg->drawSolidPoly(rect, 4, Color4F(0.f,0.f,0.f,0.35f));
    bg->drawLine(bl, br, Color4F(1,1,1,0.5f));
    bg->drawLine(br, tr, Color4F(1,1,1,0.5f));
    bg->drawLine(tr, tl, Color4F(1,1,1,0.5f));
    bg->drawLine(tl, bl, Color4F(1,1,1,0.5f));
    _hpNode->addChild(bg);

    _hpFill = DrawNode::create();
    _hpNode->addChild(_hpFill);

    _hpLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 16);
    _hpLabel->setAnchorPoint(Vec2(1,0.5f));
    _hpLabel->setPosition(Vec2(-4.0f, bh * 0.5f));
    _hpLabel->setColor(Color3B::RED);
    _hpNode->addChild(_hpLabel);

    refreshHUD();
}

void UIController::buildHotbar() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _hotbarNode = Node::create();
    _hotbarNode->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 28));
    if (_scene) _scene->addChild(_hotbarNode, 2);

    int slots = static_cast<int>(_inventory ? _inventory->size() : 0);
    float slotW = 80.0f, slotH = 32.0f, padding = 6.0f;
    float totalWidth = slots * slotW + (slots - 1) * padding;
    auto bg = DrawNode::create();
    Vec2 bgVerts[4] = { Vec2(-totalWidth/2 - 10, -slotH/2 - 8),
                        Vec2( totalWidth/2 + 10, -slotH/2 - 8),
                        Vec2( totalWidth/2 + 10,  slotH/2 + 8),
                        Vec2(-totalWidth/2 - 10,  slotH/2 + 8) };
    bg->drawSolidPoly(bgVerts, 4, Color4F(0.f, 0.f, 0.f, 0.35f));
    _hotbarNode->addChild(bg);

    _hotbarLabels.clear();
    _hotbarIcons.clear();
    for (int i = 0; i < slots; ++i) {
        float x = -totalWidth/2 + i * (slotW + padding) + slotW/2;
        auto rect = DrawNode::create();
        Vec2 r[4] = { Vec2(x - slotW/2, -slotH/2), Vec2(x + slotW/2, -slotH/2), Vec2(x + slotW/2,  slotH/2), Vec2(x - slotW/2,  slotH/2) };
        rect->drawSolidPoly(r, 4, Color4F(0.15f, 0.15f, 0.15f, 0.6f));
        _hotbarNode->addChild(rect);
        auto icon = Sprite::create();
        icon->setPosition(Vec2(x, 0));
        icon->setVisible(false);
        _hotbarNode->addChild(icon, 1);
        _hotbarIcons.push_back(icon);
        std::string text = "-";
        if (_inventory) {
            if (auto t = _inventory->toolAt(i)) {
                text = t->name;
            } else if (_inventory->isItem(i)) {
                auto st = _inventory->itemAt(i);
                text = StringUtils::format("%s x%d", Game::itemName(st.type), st.quantity);
            }
        }
        auto label = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 18);
        label->setPosition(Vec2(x, 0));
        _hotbarNode->addChild(label);
        _hotbarLabels.push_back(label);
    }

    _hotbarHighlight = DrawNode::create();
    _hotbarNode->addChild(_hotbarHighlight);
    refreshHotbar();
}

void UIController::refreshHotbar() {
    if (!_hotbarNode || !_hotbarHighlight || !_inventory) return;
    int slots = static_cast<int>(_inventory->size());
    if (slots <= 0) return;
    float slotW = 80.0f, slotH = 32.0f, padding = 6.0f;
    float totalWidth = slots * slotW + (slots - 1) * padding;
    int sel = _inventory->selectedIndex();
    float x = -totalWidth/2 + sel * (slotW + padding) + slotW/2;
    _hotbarHighlight->clear();
    Vec2 a(x - slotW/2, -slotH/2), b(x + slotW/2, -slotH/2), c(x + slotW/2,  slotH/2), d(x - slotW/2,  slotH/2);
    _hotbarHighlight->drawLine(a, b, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _hotbarHighlight->drawLine(b, c, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _hotbarHighlight->drawLine(c, d, Color4F(1.f, 0.9f, 0.2f, 1.f));
    _hotbarHighlight->drawLine(d, a, Color4F(1.f, 0.9f, 0.2f, 1.f));
    for (int i = 0; i < slots && i < static_cast<int>(_hotbarLabels.size()); ++i) {
        std::string text = "-";
        if (auto t = _inventory->toolAt(i)) {
            text = t->name;
        } else if (_inventory->isItem(i)) {
            auto st = _inventory->itemAt(i);
            text = StringUtils::format("%s x%d", Game::itemName(st.type), st.quantity);
        }
        _hotbarLabels[i]->setString(text);
        if (i < static_cast<int>(_hotbarIcons.size())) {
            auto icon = _hotbarIcons[i];
            if (_inventory->isItem(i)) {
                auto st = _inventory->itemAt(i);
                if (st.type == Game::ItemType::Fish && st.quantity > 0) {
                    auto tex = Director::getInstance()->getTextureCache()->addImage("fish/3120.png");
                    if (tex) {
                        icon->setTexture(tex);
                        float h = tex->getContentSize().height;
                        float targetH = slotH * 0.8f;
                        float scale = (h > 0) ? (targetH / h) : 1.0f;
                        icon->setScale(scale);
                        float x = -totalWidth/2 + i * (slotW + padding) + slotW/2;
                        icon->setPosition(Vec2(x, 0));
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
    }
}

void UIController::selectHotbarIndex(int idx) {
    if (_inventory) {
        _inventory->selectIndex(idx);
        Game::globalState().selectedIndex = idx;
        refreshHotbar();
        // 同步刷新水壶蓝条
        refreshWaterBar();
    }
}

bool UIController::handleHotbarMouseDown(EventMouse* e) {
    if (!_hotbarNode || !_inventory) return false;
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return false;
    auto p = e->getLocation();
    auto local = _hotbarNode->convertToNodeSpace(p);
    int slots = static_cast<int>(_inventory->size());
    if (slots <= 0) return false;
    float slotW = 80.0f, slotH = 32.0f, padding = 6.0f, hitMarginY = 8.0f;
    float totalWidth = slots * slotW + (slots - 1) * padding;
    if (local.y < -(slotH/2 + hitMarginY) || local.y > (slotH/2 + hitMarginY)) return false;
    for (int i = 0; i < slots; ++i) {
        float cx = -totalWidth/2 + i * (slotW + padding) + slotW/2;
        float minx = cx - slotW/2;
        float maxx = cx + slotW/2;
        if (local.x >= minx && local.x <= maxx) {
            selectHotbarIndex(i);
            return true;
        }
    }
    return false;
}

bool UIController::handleHotbarAtPoint(const Vec2& screenPoint) {
    if (!_hotbarNode || !_inventory) return false;
    auto local = _hotbarNode->convertToNodeSpace(screenPoint);
    int slots = static_cast<int>(_inventory->size());
    if (slots <= 0) return false;
    float slotW = 80.0f, slotH = 32.0f, padding = 6.0f, hitMarginY = 8.0f;
    float totalWidth = slots * slotW + (slots - 1) * padding;
    if (local.y < -(slotH/2 + hitMarginY) || local.y > (slotH/2 + hitMarginY)) return false;
    for (int i = 0; i < slots; ++i) {
        float cx = -totalWidth/2 + i * (slotW + padding) + slotW/2;
        float minx = cx - slotW/2;
        float maxx = cx + slotW/2;
        if (local.x >= minx && local.x <= maxx) {
            selectHotbarIndex(i);
            return true;
        }
    }
    return false;
}

void UIController::handleHotbarScroll(float dy) {
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
    // 同步刷新水壶蓝条
    refreshWaterBar();
}

bool UIController::handleChestRightClick(EventMouse* e, const std::vector<Game::Chest>& chests) {
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_RIGHT) return false;
    if (chests.empty()) return false;
    // 打开最近箱子的面板（若靠近）
    // 场景应预先根据地图检测靠近箱子；这里简化直接打开最近者
    // 查找最近的箱子
    Vec2 p = e->getLocation();
    int idx = -1; float best = 1e9f;
    for (int i=0;i<(int)chests.size();++i) {
        float d = p.distance(chests[i].pos);
        if (d < best) { best = d; idx = i; }
    }
    if (idx >= 0) {
        buildChestPanel();
        refreshChestPanel(chests[idx]);
        toggleChestPanel(true);
        return true;
    }
    return false;
}

void UIController::showDoorPrompt(bool visible, const Vec2& worldPos, const std::string& text) {
    if (!_doorPrompt) {
        _doorPrompt = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 20);
        _doorPrompt->setColor(Color3B::YELLOW);
        if (_scene) _scene->addChild(_doorPrompt, 3);
    }
    _doorPrompt->setString(text);
    _doorPrompt->setVisible(visible);
    if (visible) {
        Vec2 pos = worldPos; if (_worldNode) pos = _worldNode->convertToWorldSpace(worldPos);
        _doorPrompt->setPosition(pos + Vec2(0, 26));
    }
}

void UIController::showChestPrompt(bool visible, const Vec2& worldPos, const std::string& text) {
    if (!_chestPrompt) {
        _chestPrompt = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 20);
        _chestPrompt->setColor(Color3B::YELLOW);
        if (_scene) _scene->addChild(_chestPrompt, 3);
    }
    _chestPrompt->setString(text);
    _chestPrompt->setVisible(visible);
    if (visible) {
        Vec2 pos = worldPos; if (_worldNode) pos = _worldNode->convertToWorldSpace(worldPos);
        _chestPrompt->setPosition(pos + Vec2(0, 26));
    }
}

void UIController::showFishPrompt(bool visible, const Vec2& worldPos, const std::string& text) {
    if (!_fishPrompt) {
        _fishPrompt = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 20);
        _fishPrompt->setColor(Color3B::YELLOW);
        if (_scene) _scene->addChild(_fishPrompt, 3);
    }
    _fishPrompt->setString(text);
    _fishPrompt->setVisible(visible);
    if (visible) {
        Vec2 pos = worldPos; if (_worldNode) pos = _worldNode->convertToWorldSpace(worldPos);
        _fishPrompt->setPosition(pos + Vec2(0, 26));
    }
}

void UIController::popTextAt(const Vec2& worldPos, const std::string& text, const Color3B& color) {
    auto label = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 20);
    label->setColor(color);
    Vec2 pos = worldPos; if (_worldNode) pos = _worldNode->convertToWorldSpace(worldPos);
    label->setPosition(pos + Vec2(0, 26));
    if (_scene) _scene->addChild(label, 3);
    auto seq = Sequence::create(FadeOut::create(0.6f), RemoveSelf::create(), nullptr);
    label->runAction(seq);
}

void UIController::buildChestPanel() {
    if (_chestPanel) return;
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _chestPanel = Node::create();
    _chestPanel->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
    if (_scene) _scene->addChild(_chestPanel, 3);
    _chestPanel->setVisible(false);

    auto bg = DrawNode::create();
    float w = 360.f, h = 240.f;
    Vec2 v[4] = { Vec2(-w/2,-h/2), Vec2(w/2,-h/2), Vec2(w/2,h/2), Vec2(-w/2,h/2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f,0.f,0.f,0.55f));
    _chestPanel->addChild(bg);

    auto title = Label::createWithTTF("Chest Storage", "fonts/Marker Felt.ttf", 20);
    title->setPosition(Vec2(0, h/2 - 26));
    _chestPanel->addChild(title);

    auto closeBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(18);
    closeBtn->setScale9Enabled(true);
    closeBtn->setContentSize(Size(36, 36));
    closeBtn->setPosition(Vec2(w/2 - 20, h/2 - 20));
    closeBtn->addClickEventListener([this](Ref*){ toggleChestPanel(false); });
    _chestPanel->addChild(closeBtn);

    _chestListNode = Node::create();
    _chestPanel->addChild(_chestListNode);
}

void UIController::refreshChestPanel(const Game::Chest& chest) {
    if (!_chestPanel || !_chestListNode) return;
    _chestListNode->removeAllChildren();
    float startY = 60.f, gapY = 60.f;
    int i = 0;
    for (const auto &entry : chest.bag.all()) {
        auto t = entry.first; int have = entry.second; if (have <= 0) continue;
        float y = startY - i * gapY;
        auto nameLabel = Label::createWithTTF(Game::itemName(t), "fonts/Marker Felt.ttf", 18);
        nameLabel->setPosition(Vec2(-140, y));
        _chestListNode->addChild(nameLabel);
        auto countLabel = Label::createWithTTF(StringUtils::format("x%d", have), "fonts/Marker Felt.ttf", 18);
        countLabel->setPosition(Vec2(-60, y));
        _chestListNode->addChild(countLabel);
        ++i;
    }
}

void UIController::toggleChestPanel(bool visible) {
    if (_chestPanel) _chestPanel->setVisible(visible);
}

void UIController::buildCraftPanel() {
    if (_craftNode) return;
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _craftNode = Node::create();
    _craftNode->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
    if (_scene) _scene->addChild(_craftNode, 3);
    _craftNode->setVisible(false);
    auto bg = DrawNode::create();
    float w = 260.f, h = 180.f;
    Vec2 v[4] = { Vec2(-w/2,-h/2), Vec2(w/2,-h/2), Vec2(w/2,h/2), Vec2(-w/2,h/2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f,0.f,0.f,0.55f));
    _craftNode->addChild(bg);
    auto info = Label::createWithTTF("Chest requires 40 Wood", "fonts/Marker Felt.ttf", 18);
    info->setPosition(Vec2(0, 12));
    _craftNode->addChild(info);
    _craftButton = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    _craftButton->setTitleText("Craft Chest");
    _craftButton->setTitleFontSize(20);
    _craftButton->setScale9Enabled(true);
    _craftButton->setContentSize(Size(160, 40));
    _craftButton->setPosition(Vec2(0, -40));
    _craftNode->addChild(_craftButton);
}

void UIController::refreshCraftPanel(int woodCount) {
    if (_craftButton) {
        bool enable = woodCount >= 40;
        _craftButton->setEnabled(enable);
        _craftButton->setBright(enable);
    }
}

void UIController::toggleCraftPanel(bool visible) {
    if (_craftNode) _craftNode->setVisible(visible);
}

} // namespace Controllers

// --- Water bar (blue) above watering can slot ---
namespace Controllers {
void UIController::buildWaterBarAboveCan() {
    if (_waterBarNode) return;
    if (!_hotbarNode) return;
    _waterBarNode = Node::create();
    _hotbarNode->addChild(_waterBarNode, 3);

    _waterBarBg = DrawNode::create();
    _waterBarFill = DrawNode::create();
    _waterBarNode->addChild(_waterBarBg);
    _waterBarNode->addChild(_waterBarFill);

    float bw = 80.0f, bh = 8.0f;
    Vec2 bl(-bw/2, 0), br(bw/2, 0), tr(bw/2, bh), tl(-bw/2, bh);
    Vec2 rect[4] = { bl, br, tr, tl };
    _waterBarBg->drawSolidPoly(rect, 4, Color4F(0.f,0.f,0.f,0.35f));
    _waterBarBg->drawLine(bl, br, Color4F(1,1,1,0.4f));
    _waterBarBg->drawLine(br, tr, Color4F(1,1,1,0.4f));
    _waterBarBg->drawLine(tr, tl, Color4F(1,1,1,0.4f));
    _waterBarBg->drawLine(tl, bl, Color4F(1,1,1,0.4f));
    _waterBarNode->setVisible(false);
}

void UIController::refreshWaterBar() {
    if (!_hotbarNode || !_inventory || !_waterBarNode || !_waterBarFill) return;
    int slots = static_cast<int>(_inventory->size());
    if (slots <= 0) { _waterBarNode->setVisible(false); return; }
    int canIdx = -1;
    for (int i = 0; i < slots; ++i) {
        if (auto t = _inventory->toolAt(i)) {
            if (t->type == Game::ToolType::WateringCan) { canIdx = i; break; }
        }
    }
    if (canIdx < 0) { _waterBarNode->setVisible(false); return; }

    float slotW = 80.0f, slotH = 32.0f, padding = 6.0f;
    float totalWidth = slots * slotW + (slots - 1) * padding;
    float cx = -totalWidth/2 + canIdx * (slotW + padding) + slotW/2;
    float y = slotH/2 + 10.0f;
    _waterBarNode->setPosition(Vec2(cx, y));
    _waterBarNode->setVisible(true);

    auto &ws = Game::globalState();
    float bw = 80.0f, bh = 8.0f;
    _waterBarFill->clear();
    float ratio = ws.maxWater > 0 ? (static_cast<float>(ws.water) / static_cast<float>(ws.maxWater)) : 0.f;
    ratio = std::max(0.f, std::min(1.f, ratio));
    float fillW = bw * ratio;
    Vec2 bl(-bw/2, 0), br(-bw/2 + fillW, 0), tr(-bw/2 + fillW, bh), tl(-bw/2, bh);
    Vec2 fillRect[4] = { bl, br, tr, tl };
    _waterBarFill->drawSolidPoly(fillRect, 4, Color4F(0.2f, 0.5f, 1.0f, 0.85f));
}
}
