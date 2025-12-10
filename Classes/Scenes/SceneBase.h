/**
 * SceneBase: 提供两个场景的共性骨架，集中：
 * - worldNode 与 PlayerAppearance 的创建
 * - 共享背包加载与 UI 构建
 * - 控制器组合（PlayerController / GameStateController / 可选 ToolSystem）
 * - 通用事件转发（数字键、滚轮、热键栏点击、作弊 Z）
 *
 * 注意：业务差异（地图类型、空格交互与场景切换、提示文案）由子类覆盖对应虚方法，
 * 仍然遵守“场景只负责场景，逻辑全部在模块中”的架构。
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include "Game/Inventory.h"
#include "Game/WorldState.h"
#include "Game/View/PlayerView.h"
#include "Game/GameConfig.h"
#include "Game/Cheat.h"
#include "Controllers/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Input/PlayerController.h"
#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Systems/ToolSystem.h"
#include "Controllers/Systems/CropSystem.h"
#include <functional>
#include <vector>

class SceneBase : public cocos2d::Scene {
public:
    virtual ~SceneBase() = default;

protected:
    // 由子类在 init() 中调用：初始化共享骨架。
    bool initBase(float worldScale,
                  bool buildCraftPanel,
                  bool enableToolOnSpace,
                  bool enableToolOnLeftClick);

    // 统一 update 调度：转发到控制器并刷新提示。
    void update(float dt) override;

    // 子类必须提供：创建地图控制器；设置初始玩家位置；空格交互；提示文案。
    virtual Controllers::IMapController* createMapController(cocos2d::Node* worldNode) = 0;
    virtual void positionPlayerInitial() = 0;
    virtual void onSpacePressed() = 0;
    virtual const char* doorPromptText() const = 0;

    // 公共成员（供子类访问）：
    cocos2d::Node* _worldNode = nullptr;
    Game::IPlayerView* _player = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    Controllers::IMapController* _mapController = nullptr;
    Controllers::UIController* _uiController = nullptr;
    Controllers::PlayerController* _playerController = nullptr;
    Controllers::GameStateController* _stateController = nullptr;
    Controllers::ToolSystem* _toolSystem = nullptr;
    Controllers::CropSystem* _cropSystem = nullptr;

protected:
    // 允许子类注册额外的更新回调（用于 Monster/Mining/Combat 等控制器调度）。
    void addUpdateCallback(const std::function<void(float)>& cb);
    // 子类可覆盖的键盘钩子（默认空），用于处理自定义按键（如快速进矿洞 K）。
    virtual void onKeyPressedHook(cocos2d::EventKeyboard::KeyCode) {}

private:
    // 事件转发
    void registerCommonInputHandlers(bool enableToolOnSpace, bool enableToolOnLeftClick, bool buildCraftPanel);

protected:
    // 子类可覆盖的事件挂钩（默认空），用于转发到自定义控制器
    virtual void onMouseDown(cocos2d::EventMouse* e) {}

private:
    std::vector<std::function<void(float)>> _extraUpdates;
};
