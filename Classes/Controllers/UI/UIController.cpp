#include "Controllers/UI/UIController.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"
#include "Game/Tool/ToolBase.h"

using namespace cocos2d;

namespace Controllers {

void UIController::buildHUD() {
    if (!_hud) _hud = new HUDUI(_scene);
    _hud->buildHUD();
}

void UIController::refreshHUD() {
    if (_hud) _hud->refreshHUD();
}

void UIController::buildHPBarAboveEnergy() {
    if (!_hud) _hud = new HUDUI(_scene);
    _hud->buildHPBarAboveEnergy();
}

void UIController::buildMineFloorLabel() {
    if (!_hud) _hud = new HUDUI(_scene);
    _hud->buildMineFloorLabel();
}

void UIController::setMineFloorNumber(int floor) {
    if (_hud) _hud->setMineFloorNumber(floor);
}

void UIController::setInventoryBackground(const std::string& path) {
    if (!_hotbar) _hotbar = new HotbarUI(_scene, _inventory);
    _hotbar->setScale(2.0f);
    _hotbar->setInventoryBackground(path);
}

void UIController::buildHotbar() {
    if (!_hotbar) {
        _hotbar = new HotbarUI(_scene, _inventory);
        _hotbar->setScale(2.0f);
    }
    _hotbar->buildHotbar();
}

void UIController::refreshHotbar() { if (_hotbar) _hotbar->refreshHotbar(); }

void UIController::selectHotbarIndex(int idx) { if (_hotbar) _hotbar->selectHotbarIndex(idx); }

bool UIController::handleHotbarMouseDown(EventMouse* e) { return _hotbar ? _hotbar->handleHotbarMouseDown(e) : false; }

bool UIController::handleHotbarAtPoint(const Vec2& screenPoint) { return _hotbar ? _hotbar->handleHotbarAtPoint(screenPoint) : false; }

void UIController::handleHotbarScroll(float dy) { if (_hotbar) _hotbar->handleHotbarScroll(dy); }

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
    if (!_prompts) _prompts = new PromptUI(_scene, _worldNode);
    _prompts->showDoorPrompt(visible, worldPos, text);
}

void UIController::showChestPrompt(bool visible, const Vec2& worldPos, const std::string& text) {
    if (!_prompts) _prompts = new PromptUI(_scene, _worldNode);
    _prompts->showChestPrompt(visible, worldPos, text);
}

void UIController::showFishPrompt(bool visible, const Vec2& worldPos, const std::string& text) {
    if (!_prompts) _prompts = new PromptUI(_scene, _worldNode);
    _prompts->showFishPrompt(visible, worldPos, text);
}

void UIController::showNpcPrompt(bool visible, const Vec2& worldPos, const std::string& text) {
    if (!_prompts) _prompts = new PromptUI(_scene, _worldNode);
    _prompts->showNpcPrompt(visible, worldPos, text);
}

void UIController::popTextAt(const Vec2& worldPos, const std::string& text, const Color3B& color) {
    if (!_prompts) _prompts = new PromptUI(_scene, _worldNode);
    _prompts->popTextAt(worldPos, text, color);
}

void UIController::popFriendshipTextAt(const Vec2& worldPos, const std::string& text, const Color3B& color) {
    if (!_prompts) _prompts = new PromptUI(_scene, _worldNode);
    _prompts->popFriendshipTextAt(worldPos, text, color);
}

void UIController::buildChestPanel() {
    if (!_chestPanel) _chestPanel = new ChestPanelUI(_scene, _inventory);
    _chestPanel->buildChestPanel();
}

void UIController::refreshChestPanel(const Game::Chest& chest) { if (_chestPanel) _chestPanel->refreshChestPanel(chest); }

void UIController::toggleChestPanel(bool visible) { if (_chestPanel) _chestPanel->toggleChestPanel(visible); }

void UIController::buildCraftPanel() { if (!_craftPanel) _craftPanel = new CraftPanelUI(_scene); _craftPanel->buildCraftPanel(); }

void UIController::refreshCraftPanel(int woodCount) { if (_craftPanel) _craftPanel->refreshCraftPanel(woodCount); }

void UIController::toggleCraftPanel(bool visible) { if (_craftPanel) _craftPanel->toggleCraftPanel(visible); }

} // namespace Controllers

// Water bar UI removed; overlay managed by tool implementations.

namespace Controllers {

void UIController::buildStorePanel() {
    if (!_storePanel) _storePanel = new StorePanelUI(_scene, _inventory);
    _storePanel->buildStorePanel();
    _storePanel->onPurchased = [this](bool ok){
        if (ok) {
            refreshHUD();
            refreshHotbar();
            popTextAt(_scene->convertToWorldSpace(Vec2(0,0)), "Bought!", Color3B::GREEN);
        } else {
            popTextAt(_scene->convertToWorldSpace(Vec2(0,0)), "Failed", Color3B::RED);
        }
    };
}

void UIController::refreshStorePanel() { if (_storePanel) _storePanel->refreshStorePanel(); }

void UIController::toggleStorePanel(bool visible) {
    if (visible) {
        buildStorePanel();
        refreshStorePanel();
        if (_storePanel) _storePanel->toggleStorePanel(true);
    } else {
        if (_storePanel) _storePanel->toggleStorePanel(false);
    }
}

}
