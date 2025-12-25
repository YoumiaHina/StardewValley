// UIController：
// - 作用：统一管理 HUD、热键栏、提示文本与各类面板 UI 的创建、刷新与显示隐藏。
// - 职责边界：只负责 UI 节点与输入分发，不承载背包/交易/技能等业务规则。
// - 主要协作对象：Inventory 提供物品数据；地图/交互模块提供世界坐标与交互目标；各 *PanelUI 负责具体面板绘制。
#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "Game/Inventory.h"
#include "Game/PlaceableItem/Chest.h"
#include "Game/Animals/Animal.h"
#include "Controllers/UI/HUDUI.h"
#include "Controllers/UI/HotbarUI.h"
#include "Controllers/UI/PromptUI.h"
#include "Controllers/UI/ChestPanelUI.h"
#include "Controllers/UI/CraftPanelUI.h"
#include "Controllers/UI/StorePanelUI.h"
#include "Controllers/UI/SkillTreePanelUI.h"
#include "Controllers/UI/ToolUpgradePanelUI.h"
#include "Controllers/UI/DialogueUI.h"
#include "Controllers/UI/NpcSocialPanelUI.h"
#include "Controllers/UI/ElevatorPanelUI.h"

namespace Controllers {

// UI 控制器：对外提供“一站式 UI 能力入口”，并持有各子 UI 的生命周期。
class UIController {
public:
    // 构造：注入场景、世界节点与背包；UI 节点在各 build* 中按需创建。
    UIController(cocos2d::Scene* scene,
                 cocos2d::Node* worldNode,
                 std::shared_ptr<Game::Inventory> inventory)
    : _scene(scene), _worldNode(worldNode), _inventory(inventory) {}
    // 析构：释放各子 UI 对象（不拥有 scene/worldNode/inventory 的生命周期）。
    ~UIController() {
        delete _hud; _hud = nullptr;
        delete _hotbar; _hotbar = nullptr;
        delete _prompts; _prompts = nullptr;
        delete _chestPanel; _chestPanel = nullptr;
        delete _craftPanel; _craftPanel = nullptr;
        delete _storePanel; _storePanel = nullptr;
        delete _skillTreePanel; _skillTreePanel = nullptr;
        delete _toolUpgradePanel; _toolUpgradePanel = nullptr;
        delete _animalStorePanel; _animalStorePanel = nullptr;
        delete _dialogueUI; _dialogueUI = nullptr;
        delete _socialPanel; _socialPanel = nullptr;
        delete _elevatorPanel; _elevatorPanel = nullptr;
    }

    // 构建 HUD（时间/天气/金币/体力等）。
    void buildHUD();
    // 刷新 HUD 内容显示。
    void refreshHUD();

    // 构建热键栏。
    void buildHotbar();
    // 刷新热键栏内容显示。
    void refreshHotbar();
    // 选中指定热键栏索引并刷新高亮。
    void selectHotbarIndex(int idx);

    // 设置背包背景贴图路径（用于热键栏背景）。
    void setInventoryBackground(const std::string& path);


    // 处理热键栏鼠标按下事件：返回是否命中并被消费。
    bool handleHotbarMouseDown(cocos2d::EventMouse* e);
    // 处理箱子右键：尝试打开对应的箱子面板；返回是否命中并被消费。
    bool handleChestRightClick(cocos2d::EventMouse* e, std::vector<Game::Chest>& chests);
    // 判断屏幕坐标是否命中热键栏区域：返回是否命中。
    bool handleHotbarAtPoint(const cocos2d::Vec2& screenPoint);
    // 处理热键栏滚轮切换：dy 为滚轮增量。
    void handleHotbarScroll(float dy);
    // 矿洞专属：在能量条正上方绘制血条（红色）。
    void buildHPBarAboveEnergy();
    // 矿洞楼层标签（左上角显示当前层数）。
    void buildMineFloorLabel();
    // 设置矿洞楼层数字显示。
    void setMineFloorNumber(int floor);

    // 显示/隐藏门口提示（传入世界坐标）。
    void showDoorPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    // 显示/隐藏箱子提示（传入世界坐标）。
    void showChestPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    // 显示/隐藏钓鱼提示（传入世界坐标）。
    void showFishPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    // 显示/隐藏 NPC 交互提示（传入世界坐标）。
    void showNpcPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);

    // 弹出短文本。
    void popTextAt(const cocos2d::Vec2& worldPos, const std::string& text, const cocos2d::Color3B& color);
    // 弹出好感度变化文本。
    void popFriendshipTextAt(const cocos2d::Vec2& worldPos, const std::string& text, const cocos2d::Color3B& color);
    // 弹出屏幕中心大文本。
    void popCenterBigText(const std::string& text, const cocos2d::Color3B& color);

    // 显示对话面板：注入文本、选项与回调。
    void showDialogue(const std::string& npcName,
                      const std::string& text,
                      const std::vector<std::string>& options,
                      std::function<void(int)> onOption,
                      std::function<void()> onAdvance);
    // 隐藏对话面板。
    void hideDialogue();
    // 对话面板是否可见。
    bool isDialogueVisible() const;

    // 显示 NPC 社交面板。
    void showNpcSocial(int npcKey, const std::string& npcName);
    // 隐藏 NPC 社交面板。
    void hideNpcSocial();
    // NPC 社交面板是否可见。
    bool isNpcSocialVisible() const;

    // 构建箱子面板。
    void buildChestPanel();
    // 刷新箱子面板内容（绑定当前箱子并重绘格子）。
    void refreshChestPanel(Game::Chest* chest);
    // 显示/隐藏箱子面板。
    void toggleChestPanel(bool visible);
    // 箱子面板是否可见。
    bool isChestPanelVisible() const;

    // Craft 面板
    // 构建合成面板。
    void buildCraftPanel();
    // 显示/隐藏合成面板。
    void toggleCraftPanel(bool visible);
    // 合成面板是否可见。
    bool isCraftPanelVisible() const;

    // Store 面板
    // 构建商店面板。
    void buildStorePanel();
    // 刷新商店面板内容。
    void refreshStorePanel();
    // 显示/隐藏商店面板。
    void toggleStorePanel(bool visible);
    // 商店面板是否可见。
    bool isStorePanelVisible() const;

    // 构建动物商店面板。
    void buildAnimalStorePanel();
    // 刷新动物商店面板内容。
    void refreshAnimalStorePanel();
    // 显示/隐藏动物商店面板。
    void toggleAnimalStorePanel(bool visible);
    // 动物商店面板是否可见。
    bool isAnimalStorePanelVisible() const;
    // 设置动物购买处理回调（扣费/生成等由上层实现）。
    void setAnimalStoreHandler(const std::function<bool(Game::AnimalType)>& handler);

    // 技能等级面板：构建 UI 节点。
    void buildSkillTreePanel();
    // 技能等级面板：刷新展示内容。
    void refreshSkillTreePanel();
    // 技能等级面板：显示/隐藏。
    void toggleSkillTreePanel(bool visible);
    // 技能等级面板是否可见。
    bool isSkillTreePanelVisible() const;

    // 构建工具升级面板。
    void buildToolUpgradePanel();
    // 刷新工具升级面板内容。
    void refreshToolUpgradePanel();
    // 显示/隐藏工具升级面板。
    void toggleToolUpgradePanel(bool visible);
    // 工具升级面板是否可见。
    bool isToolUpgradePanelVisible() const;

    // 电梯面板：构建 UI 节点（只负责创建与挂载到场景）
    void buildElevatorPanel();
    // 电梯面板：根据可达楼层列表刷新按钮
    void refreshElevatorPanel(const std::vector<int>& floors);
    // 电梯面板：显示/隐藏
    void toggleElevatorPanel(bool visible);
    // 电梯面板是否可见
    bool isElevatorPanelVisible() const;
    // 设置电梯楼层回调：当玩家在面板中选择楼层时触发
    void setElevatorFloorHandler(const std::function<void(int)>& cb);

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
    ToolUpgradePanelUI* _toolUpgradePanel = nullptr;
    AnimalStorePanelUI* _animalStorePanel = nullptr;
    DialogueUI* _dialogueUI = nullptr;
    NpcSocialPanelUI* _socialPanel = nullptr;
    // 电梯楼层选择面板
    ElevatorPanelUI* _elevatorPanel = nullptr;
    std::function<bool(Game::AnimalType)> _animalStoreHandler;
};

}
// namespace Controllers
