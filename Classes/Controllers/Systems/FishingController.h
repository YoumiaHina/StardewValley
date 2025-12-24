#pragma once

#include "cocos2d.h"
#include <memory>
#include "Controllers/Map/IMapController.h"
#include "Game/Inventory.h"
#include "Controllers/UI/UIController.h"

namespace Controllers {

// 钓鱼小游戏控制器：
// - 负责在世界坐标某点创建钓鱼 UI 覆盖层，并驱动“鱼与绿色条”的物理解算。
// - 通过 Inventory 发放鱼类奖励，并与 SkillTreeSystem/WorldState 协作更新经验与状态。
// - startAt 会检查是否在湖边；startAnywhere 则忽略湖泊约束，主要用于节日活动。
class FishingController {
public:
    FishingController(Controllers::IMapController* map,
                      std::shared_ptr<Game::Inventory> inventory,
                      Controllers::UIController* ui,
                      cocos2d::Scene* scene,
                      cocos2d::Node* worldNode)
    : _map(map), _inventory(std::move(inventory)), _ui(ui), _scene(scene), _worldNode(worldNode) {}

    // 在湖边开始一次钓鱼；若不在可钓区域或处于冷却中则直接返回。
    void startAt(const cocos2d::Vec2& worldPos);
    // 任意位置开始钓鱼（忽略湖泊检查），用于节日小游戏等特殊场景。
    void startAnywhere(const cocos2d::Vec2& worldPos);
    // 驱动钓鱼物理及 UI 绘制，每帧调用。
    void update(float dt);
    bool isActive() const { return _active; }
    // 主动取消当前钓鱼过程（例如玩家离开区域或中断动作）。
    void cancel();
    // 设置一个回调，用于在钓鱼开始/结束时锁定或解锁玩家移动。
    void setMovementLocker(std::function<void(bool)> cb) { _setMovementLocked = std::move(cb); }

private:
    Controllers::IMapController* _map = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    Controllers::UIController* _ui = nullptr;
    cocos2d::Scene* _scene = nullptr;
    cocos2d::Node* _worldNode = nullptr;

    cocos2d::Node* _overlay = nullptr;
    cocos2d::DrawNode* _barBg = nullptr;
    cocos2d::DrawNode* _barCatch = nullptr;
    cocos2d::DrawNode* _progressFill = nullptr;
    cocos2d::Label* _progressLabel = nullptr;
    cocos2d::Sprite* _bgSprite = nullptr;
    cocos2d::Sprite* _fishSprite = nullptr;

    bool _active = false;
    int _festivalFishIndex = -1;
    float _barHeight = 220.0f;
    float _barCatchPos = 80.0f;
    float _barCatchVel = 0.0f;
    float _fishPos = 140.0f;
    float _fishVel = 0.0f;
    float _progress = 0.0f; // 0..100
    float _timeLeft = 12.0f; // seconds
    bool _hold = false; // input hold

    float _cooldown = 0.0f;

    cocos2d::EventListenerMouse* _mouse = nullptr;
    std::function<void(bool)> _setMovementLocked;

    void buildOverlayAt(const cocos2d::Vec2& worldPos);
    void destroyOverlay();
    void onSuccess(const cocos2d::Vec2& worldPos);
    void onFail(const cocos2d::Vec2& worldPos);
};

}
// namespace Controllers
