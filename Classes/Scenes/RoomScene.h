/**
 * RoomScene: 玩家出生在室内，走到门口自动切换到农场（GameScene）。
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "Game/Inventory.h"

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
    cocos2d::DrawNode* _player = nullptr;
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
};