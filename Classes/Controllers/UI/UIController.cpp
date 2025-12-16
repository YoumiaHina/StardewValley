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

bool UIController::handleHotbarMouseDown(EventMouse* e) {
    if (!_hotbar || !_inventory) return false;
    bool handled = _hotbar->handleHotbarMouseDown(e);
    if (!handled) return false;
    if (_chestPanel && _chestPanel->isVisible()) {
        int idx = _inventory->selectedIndex();
        _chestPanel->onInventorySlotClicked(idx);
    }
    return true;
}

bool UIController::handleHotbarAtPoint(const Vec2& screenPoint) {
    if (!_hotbar || !_inventory) return false;
    bool handled = _hotbar->handleHotbarAtPoint(screenPoint);
    if (!handled) return false;
    if (_chestPanel && _chestPanel->isVisible()) {
        int idx = _inventory->selectedIndex();
        _chestPanel->onInventorySlotClicked(idx);
    }
    return true;
}

void UIController::handleHotbarScroll(float dy) { if (_hotbar) _hotbar->handleHotbarScroll(dy); }

bool UIController::handleChestRightClick(EventMouse* e, std::vector<Game::Chest>& chests) {
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_RIGHT) return false;
    if (chests.empty()) return false;
    Vec2 p = e->getLocation();
    int idx = -1;
    float best = 1e9f;
    for (int i=0;i<(int)chests.size();++i) {
        Vec2 center = chests[i].pos;
        Rect clickRect = Game::chestRect(chests[i]);
        if (clickRect.containsPoint(p)) {
            float d = p.distance(center);
            if (d < best) {
                best = d;
                idx = i;
            }
        }
    }
    if (idx >= 0) {
        buildChestPanel();
        refreshChestPanel(&chests[idx]);
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

void UIController::showNpcSocial(int npcKey, const std::string& npcName) {
    if (!_socialPanel) _socialPanel = new NpcSocialPanelUI(_scene);
    auto& ws = Game::globalState();
    int friendship = 0;
    auto it = ws.npcFriendship.find(npcKey);
    if (it != ws.npcFriendship.end()) friendship = it->second;
    bool romance = false;
    auto itR = ws.npcRomanceUnlocked.find(npcKey);
    if (itR != ws.npcRomanceUnlocked.end()) romance = itR->second;
    const std::vector<Game::NpcQuest>* quests = nullptr;
    auto itQ = ws.npcQuests.find(npcKey);
    if (itQ != ws.npcQuests.end()) quests = &itQ->second;
    _socialPanel->show(npcName, friendship, romance, quests);
}

void UIController::hideNpcSocial() {
    if (_socialPanel) _socialPanel->hide();
}

bool UIController::isNpcSocialVisible() const {
    return _socialPanel && _socialPanel->isVisible();
}

void UIController::showDialogue(const std::string& npcName,
                                const std::string& text,
                                const std::vector<std::string>& options,
                                std::function<void(int)> onOption,
                                std::function<void()> onAdvance) {
    if (!_dialogueUI) _dialogueUI = new DialogueUI(_scene);
    _dialogueUI->show(npcName, text, options, std::move(onOption), std::move(onAdvance));
}

void UIController::hideDialogue() {
    if (_dialogueUI) _dialogueUI->hide();
}

bool UIController::isDialogueVisible() const {
    return _dialogueUI && _dialogueUI->isVisible();
}

void UIController::buildChestPanel() {
    if (!_chestPanel) {
        _chestPanel = new ChestPanelUI(_scene, _inventory);
        _chestPanel->setOnInventoryChanged([this](){
            refreshHotbar();
        });
    }
    _chestPanel->buildChestPanel();
}

void UIController::refreshChestPanel(Game::Chest* chest) {
    if (_chestPanel) _chestPanel->refreshChestPanel(chest);
}

void UIController::toggleChestPanel(bool visible) { if (_chestPanel) _chestPanel->toggleChestPanel(visible); }

bool UIController::isChestPanelVisible() const {
    return _chestPanel && _chestPanel->isVisible();
}

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

bool UIController::isStorePanelVisible() const {
    return _storePanel && _storePanel->isVisible();
}

void UIController::buildAnimalStorePanel() {
    if (!_animalStorePanel) _animalStorePanel = new AnimalStorePanelUI(_scene);
    _animalStorePanel->buildAnimalStorePanel();
    _animalStorePanel->onPurchased = [this](bool ok){
        if (ok) {
            refreshHUD();
            refreshHotbar();
            popTextAt(_scene->convertToWorldSpace(Vec2(0,0)), "Bought!", Color3B::GREEN);
        } else {
            popTextAt(_scene->convertToWorldSpace(Vec2(0,0)), "Failed", Color3B::RED);
        }
    };
    _animalStorePanel->onBuyAnimal = [this](Game::AnimalType type) -> bool {
        if (_animalStoreHandler) {
            return _animalStoreHandler(type);
        }
        return false;
    };
}

void UIController::refreshAnimalStorePanel() {
    if (_animalStorePanel) _animalStorePanel->refreshAnimalStorePanel();
}

void UIController::toggleAnimalStorePanel(bool visible) {
    if (visible) {
        buildAnimalStorePanel();
        refreshAnimalStorePanel();
        if (_animalStorePanel) _animalStorePanel->toggleAnimalStorePanel(true);
    } else {
        if (_animalStorePanel) _animalStorePanel->toggleAnimalStorePanel(false);
    }
}

bool UIController::isAnimalStorePanelVisible() const {
    return _animalStorePanel && _animalStorePanel->isVisible();
}

void UIController::setAnimalStoreHandler(const std::function<bool(Game::AnimalType)>& handler) {
    _animalStoreHandler = handler;
}

}
