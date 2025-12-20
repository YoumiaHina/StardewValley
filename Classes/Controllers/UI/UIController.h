/**
 * UIController: 负责 HUD（时间/能量）、热键栏、提示（门/箱子）、面板（箱子/合成）与弹出文本。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include <functional>
#include "Game/Inventory.h"
#include "Game/Chest.h"
#include "Controllers/UI/HUDUI.h"
#include "Controllers/UI/HotbarUI.h"
#include "Controllers/UI/PromptUI.h"
#include "Controllers/UI/ChestPanelUI.h"
#include "Controllers/UI/CraftPanelUI.h"
#include "Controllers/UI/StorePanelUI.h"
#include "Controllers/UI/SkillTreePanelUI.h"
#include "Controllers/UI/DialogueUI.h"
#include "Controllers/UI/NpcSocialPanelUI.h"
#include "ui/CocosGUI.h"

namespace Controllers {

class UIController {
public:
    UIController(cocos2d::Scene* scene,
                 cocos2d::Node* worldNode,
                 std::shared_ptr<Game::Inventory> inventory)
    : _scene(scene), _worldNode(worldNode), _inventory(inventory) {}
    ~UIController() {
        delete _hud; _hud = nullptr;
        delete _hotbar; _hotbar = nullptr;
        delete _prompts; _prompts = nullptr;
        delete _chestPanel; _chestPanel = nullptr;
        delete _craftPanel; _craftPanel = nullptr;
        delete _storePanel; _storePanel = nullptr;
        delete _skillTreePanel; _skillTreePanel = nullptr;
        delete _dialogueUI; _dialogueUI = nullptr;
        delete _socialPanel; _socialPanel = nullptr;
    }

    void buildHUD();
    void refreshHUD();

    void buildHotbar();
    void refreshHotbar();
    void selectHotbarIndex(int idx);

    void setInventoryBackground(const std::string& path);


    bool handleHotbarMouseDown(cocos2d::EventMouse* e);
    bool handleChestRightClick(cocos2d::EventMouse* e, std::vector<Game::Chest>& chests);
    bool handleHotbarAtPoint(const cocos2d::Vec2& screenPoint);
    void handleHotbarScroll(float dy);
    // 矿洞专属：在能量条正上方绘制血条（红色）。
    void buildHPBarAboveEnergy();
    // 矿洞楼层标签（左上角显示当前层数）
    void buildMineFloorLabel();
    void setMineFloorNumber(int floor);

    // 提示显示/隐藏（传入世界坐标）
    void showDoorPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    void showChestPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    void showFishPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    void showNpcPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);

    // 弹出短文本
    void popTextAt(const cocos2d::Vec2& worldPos, const std::string& text, const cocos2d::Color3B& color);
    void popFriendshipTextAt(const cocos2d::Vec2& worldPos, const std::string& text, const cocos2d::Color3B& color);

    void showDialogue(const std::string& npcName,
                      const std::string& text,
                      const std::vector<std::string>& options,
                      std::function<void(int)> onOption,
                      std::function<void()> onAdvance);
    void hideDialogue();
    bool isDialogueVisible() const;

    void showNpcSocial(int npcKey, const std::string& npcName);
    void hideNpcSocial();
    bool isNpcSocialVisible() const;

    void buildChestPanel();
    void refreshChestPanel(Game::Chest* chest);
    void toggleChestPanel(bool visible);
    bool isChestPanelVisible() const;

    // Craft 面板（简化：显示按钮与可用状态）
    void buildCraftPanel();
    void refreshCraftPanel(int woodCount);
    void toggleCraftPanel(bool visible);

    // Store 面板
    void buildStorePanel();
    void refreshStorePanel();
    void toggleStorePanel(bool visible);
    bool isStorePanelVisible() const;

    void buildAnimalStorePanel();
    void refreshAnimalStorePanel();
    void toggleAnimalStorePanel(bool visible);
    bool isAnimalStorePanelVisible() const;
    void setAnimalStoreHandler(const std::function<bool(Game::AnimalType)>& handler);

    // 技能等级面板：构建 UI 节点。
    void buildSkillTreePanel();
    // 技能等级面板：刷新展示内容。
    void refreshSkillTreePanel();
    // 技能等级面板：显示/隐藏。
    void toggleSkillTreePanel(bool visible);
    // 技能等级面板是否可见。
    bool isSkillTreePanelVisible() const;

private:
    cocos2d::Scene* _scene = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    HUDUI* _hud = nullptr;
    HotbarUI* _hotbar = nullptr;
    PromptUI* _prompts = nullptr;
    ChestPanelUI* _chestPanel = nullptr;
    CraftPanelUI* _craftPanel = nullptr;
    StorePanelUI* _storePanel = nullptr;
    SkillTreePanelUI* _skillTreePanel = nullptr;
    AnimalStorePanelUI* _animalStorePanel = nullptr;
    DialogueUI* _dialogueUI = nullptr;
    NpcSocialPanelUI* _socialPanel = nullptr;
    std::function<bool(Game::AnimalType)> _animalStoreHandler;
};

}
// namespace Controllers
