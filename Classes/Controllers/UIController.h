/**
 * UIController: 负责 HUD（时间/能量）、热键栏、提示（门/箱子）、面板（箱子/合成）与弹出文本。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include "Game/Inventory.h"
#include "Game/Chest.h"
#include "ui/CocosGUI.h"

namespace Controllers {

class UIController {
public:
    UIController(cocos2d::Scene* scene,
                 cocos2d::Node* worldNode,
                 std::shared_ptr<Game::Inventory> inventory)
    : _scene(scene), _worldNode(worldNode), _inventory(std::move(inventory)) {}

    void buildHUD();
    void refreshHUD();

    void buildHotbar();
    void refreshHotbar();
    void selectHotbarIndex(int idx);

    // 事件转发：处理热键栏点击选择与右键打开箱子面板
    bool handleHotbarMouseDown(cocos2d::EventMouse* e);
    bool handleChestRightClick(cocos2d::EventMouse* e, const std::vector<Game::Chest>& chests);
    bool handleHotbarAtPoint(const cocos2d::Vec2& screenPoint);
    void handleHotbarScroll(float dy);
    // 矿洞专属：在能量条正上方绘制血条（红色）。
    void buildHPBarAboveEnergy();

    // 提示显示/隐藏（传入世界坐标）
    void showDoorPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    void showChestPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);

    // 弹出短文本
    void popTextAt(const cocos2d::Vec2& worldPos, const std::string& text, const cocos2d::Color3B& color);

    // Chest 面板（简化为只显示/刷新列表）
    void buildChestPanel();
    void refreshChestPanel(const Game::Chest& chest);
    void toggleChestPanel(bool visible);

    // Craft 面板（简化：显示按钮与可用状态）
    void buildCraftPanel();
    void refreshCraftPanel(int woodCount);
    void toggleCraftPanel(bool visible);

private:
    cocos2d::Scene* _scene = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;

    // HUD
    cocos2d::Label* _hudTimeLabel = nullptr;
    cocos2d::Node* _energyNode = nullptr;
    cocos2d::DrawNode* _energyFill = nullptr;
    cocos2d::Label* _energyLabel = nullptr;
    // HP HUD（矿洞专属）
    cocos2d::Node* _hpNode = nullptr;
    cocos2d::DrawNode* _hpFill = nullptr;
    cocos2d::Label* _hpLabel = nullptr;

    // Hotbar
    cocos2d::Node* _hotbarNode = nullptr;
    cocos2d::DrawNode* _hotbarHighlight = nullptr;
    std::vector<cocos2d::Label*> _hotbarLabels;

    // Prompts
    cocos2d::Label* _doorPrompt = nullptr;
    cocos2d::Label* _chestPrompt = nullptr;

    // Chest Panel
    cocos2d::Node* _chestPanel = nullptr;
    cocos2d::Node* _chestListNode = nullptr;

    // Craft Panel
    cocos2d::Node* _craftNode = nullptr;
    cocos2d::ui::Button* _craftButton = nullptr;
};

} // namespace Controllers