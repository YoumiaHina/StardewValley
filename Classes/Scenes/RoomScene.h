/**
 * RoomScene: 玩家出生在室内，走到门口自动切换到农场（GameScene）。
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "Game/Inventory.h"
#include "Game/Chest.h"
// 需要使用 UI Button
#include "ui/CocosGUI.h"

#include "Game/PlayerAppearance.h"

class RoomScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(RoomScene);

    void update(float dt) override;
    // 设置在门内侧的出生点（从农场返回时）
    void setSpawnInsideDoor();
    // 构建并刷新室内热键栏（物品栏）
    void buildHotbarUI();
    void refreshHotbarUI();
    // HUD（时间与能量）
    void buildHUD();
    void refreshHUD();

private:
    // 玩家与房间绘制
    cocos2d::Node* _worldNode = nullptr; // Scale container
    Game::PlayerAppearance* _player = nullptr;
    cocos2d::DrawNode* _roomDraw = nullptr;

    // 房间与门区域
    cocos2d::Rect _roomRect;   // 房间有效移动范围
    cocos2d::Rect _doorRect;   // 门口区域
    bool _transitioning = false; // 防止重复触发场景切换
    cocos2d::Label* _doorPrompt = nullptr; // 交互提示（靠近门口显示）
    bool _nearDoor = false;                // 是否靠近门口
    // 摆设
    cocos2d::Rect _bedRect;
    cocos2d::Rect _tableRect;
    cocos2d::Label* _bedLabel = nullptr;
    cocos2d::Label* _tableLabel = nullptr;
    cocos2d::Label* _bedPrompt = nullptr;  // 靠近床的交互提示
    bool _nearBed = false;

    // 物品栏（与 GameScene 共享同一实例）
    std::shared_ptr<Game::Inventory> _inventory;
    cocos2d::Node* _hotbarNode = nullptr;
    cocos2d::DrawNode* _hotbarHighlight = nullptr;
    std::vector<cocos2d::Label*> _hotbarLabels;

    // HUD：时间与能量
    cocos2d::Label* _hudTimeLabel = nullptr;
    cocos2d::Node* _energyNode = nullptr;
    cocos2d::DrawNode* _energyFill = nullptr;
    cocos2d::Label* _energyLabel = nullptr;

    // 室内箱子：数据与绘制
    std::vector<Game::Chest> _houseChests;
    cocos2d::DrawNode* _chestDraw = nullptr;
    cocos2d::Label* _chestPrompt = nullptr;
    bool _nearChest = false;

    // 输入与移动参数（与 GameScene 保持一致体验）
    bool _up = false;
    bool _down = false;
    bool _left = false;
    bool _right = false;
    float _baseSpeed = 140.0f;
    float _sprintSpeed = 240.0f;
    float _moveHeldDuration = 0.0f;
    bool  _isSprinting = false;
    const float _sprintThreshold = 0.5f; // 长按任意移动键 0.5 秒后加速

    void buildRoom();
    void checkDoorRegion();
    void checkBedRegion();
    void refreshChestsVisuals();
    void checkChestRegion();

    // 室内箱子面板与交互
    void buildChestUI();
    void refreshChestUI();
    void showChestPanel(int idx);
    void attemptWithdraw(Game::ItemType type, int qty);

    // 面板数据与拖拽状态
    cocos2d::Node* _chestPanel = nullptr;
    cocos2d::Node* _chestListNode = nullptr;
    int _activeChestIdx = -1;
    struct WithdrawRow {
        Game::ItemType type;
        cocos2d::Label* nameLabel = nullptr;
        cocos2d::Label* countLabel = nullptr;
        cocos2d::Label* planLabel = nullptr;
        cocos2d::ui::Button* minusBtn = nullptr;
        cocos2d::ui::Button* plusBtn = nullptr;
        cocos2d::ui::Button* takeBtn = nullptr;
        int planQty = 0;
    };
    std::vector<WithdrawRow> _withdrawRows;
    float _chestPanelW = 360.f;
    float _chestPanelH = 240.f;
    float _chestRowStartY = 60.f;
    float _chestRowGapY = 60.f;
    enum class DragSource { None, Inventory, Chest };
    DragSource _dragSource = DragSource::None;
    bool _dragging = false;
    int _dragSlotIndex = -1;
    Game::ItemType _dragType;
    int _dragQty = 0;
    cocos2d::Label* _dragGhost = nullptr;
};