/**
 * SceneBase：场景基类（唯一允许的场景继承基类）。
 * - 作用：搭建通用场景骨架（worldNode、玩家视图、UI、通用控制器组合与更新调度）。
 * - 职责边界：只做“骨架 + 事件转发 + 调度”，不承载具体业务规则；差异点由子类覆盖虚接口。
 * - 协作对象：通过 Controllers::IMapController 与 PlayerController/UIController/GameStateController 等模块协作。
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include "Game/Inventory.h"
#include "Game/WorldState.h"
#include "Game/View/PlayerView.h"
#include "Game/GameConfig.h"
#include "Game/Cheat.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Input/PlayerController.h"
#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Systems/CropSystem.h"
#include <functional>
#include <vector>

namespace Controllers { class FishingController; }
namespace Controllers { class WeatherController; }
namespace Controllers { class FestivalController; }

class SceneBase : public cocos2d::Scene {
public:
    // 析构：释放场景持有的控制器与资源。
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
    // 创建地图控制器：返回当前场景使用的 IMapController 实现。
    virtual Controllers::IMapController* createMapController(cocos2d::Node* worldNode) = 0;
    // 设置玩家初始位置：按全局状态/场景自有出生点规则放置玩家。
    virtual void positionPlayerInitial() = 0;
    // 空格交互：由子类决定具体交互（进门/交互面板等），业务逻辑委派给模块。
    virtual void onSpacePressed() = 0;
    // 门口提示文案：由子类提供不同场景的提示文本。
    virtual const char* doorPromptText() const = 0;

    // 场景级额外的移动锁定条件（默认无）。子类可覆盖，用于在自定义面板/流程
    // 打开时禁止玩家移动，例如矿洞电梯面板。
    // 是否需要由场景额外阻止玩家移动。
    virtual bool isMovementBlockedByScene() const { return false; }

    // 公共成员（供子类访问）：
    cocos2d::Node* _worldNode = nullptr;
    Game::IPlayerView* _player = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    Controllers::IMapController* _mapController = nullptr;
    Controllers::UIController* _uiController = nullptr;
    Controllers::PlayerController* _playerController = nullptr;
    Controllers::GameStateController* _stateController = nullptr;
    Controllers::CropSystem* _cropSystem = nullptr;
    Controllers::FishingController* _fishingController = nullptr;
    Controllers::WeatherController* _weatherController = nullptr;
    Controllers::FestivalController* _festivalController = nullptr;

    // 允许子类注册额外的更新回调（用于 Monster/Mining/Combat 等控制器调度）。
    void addUpdateCallback(const std::function<void(float)>& cb);
    // 子类可覆盖的键盘钩子（默认空），用于处理自定义按键（如快速进矿洞 K）。
    // 键盘事件钩子：提供给子类做额外按键处理（仅转发，不写业务规则）。
    virtual void onKeyPressedHook(cocos2d::EventKeyboard::KeyCode) {}

private:
    // 事件转发
    // 注册通用输入处理并绑定到 PlayerController/子类钩子。
    void registerCommonInputHandlers(bool enableToolOnSpace, bool enableToolOnLeftClick, bool buildCraftPanel);

protected:
    // 子类可覆盖的事件挂钩（默认空），用于转发到自定义控制器
    // 鼠标按下事件钩子：提供给子类转发到交互/战斗等模块。
    virtual void onMouseDown(cocos2d::EventMouse* e) {}

private:
    std::vector<std::function<void(float)>> _extraUpdates;
    cocos2d::LayerColor* _dayNightOverlay = nullptr;
};
