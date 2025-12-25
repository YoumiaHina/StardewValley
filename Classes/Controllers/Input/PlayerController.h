// PlayerController：玩家输入与移动控制器。
// - 职责：
//   1. 统一采集键盘/鼠标/触摸输入，转换为玩家移动、冲刺与工具使用请求。
//   2. 根据输入状态驱动 IPlayerView 的朝向/动画，以及与地图控制器协作完成移动与相机跟随。
//   3. 协调 UIController/Inventory/CropSystem 等上层模块的快捷键行为（热键栏、技能面板、锄地/浇水等）。
// - 职责边界：
//   - 不直接维护地图/作物/箱子等业务状态，只通过 IMapController/CropSystem/ChestController 等接口协作。
//   - 不做存档格式与具体 UI 细节的管理，这些由 SaveSystem 与各 UI 控制器负责。
// - 协作对象：
//   - Game::IPlayerView：负责玩家精灵位置、朝向与动画播放。
//   - Controllers::IMapController：负责坐标换算、碰撞与环境排序以及光标更新。
//   - Controllers::UIController：负责 HUD/面板的开关与热键栏交互。
//   - Game::Inventory / Controllers::CropSystem 等：由工具逻辑与快捷键调用。
#pragma once

#include "cocos2d.h"
#include <functional>
#include <memory>
#include "Controllers/Map/IMapController.h"

namespace Game { class IPlayerView; }
namespace Game { class Inventory; }

namespace Controllers {

class UIController;
class CropSystem;

// PlayerController：封装玩家输入/移动/工具触发的控制逻辑。
// - 由具体场景持有，在场景 init 时绑定 IMapController/UI/Inventory 等依赖。
class PlayerController {
public:
    // 构造函数：绑定玩家视图、地图控制器与世界节点，并设置基础/冲刺速度参数。
    PlayerController(Game::IPlayerView* player,
                     IMapController* map,
                     cocos2d::Node* worldNode,
                     float baseSpeed = 140.0f,
                     float sprintSpeed = 240.0f,
                     float sprintThreshold = 0.5f)
    : _player(player), _map(map), _worldNode(worldNode),
      _baseSpeed(baseSpeed), _sprintSpeed(sprintSpeed), _sprintThreshold(sprintThreshold) {}

    // 处理键盘按下事件：更新方向键状态与冲刺/工具范围修正标记。
    void onKeyPressed(cocos2d::EventKeyboard::KeyCode code);
    // 处理键盘抬起事件：清除方向键状态与工具范围修正标记。
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode code);

    // 注册常用输入监听：在 ownerNode 上挂接键盘/鼠标/触摸事件，并与 UI/背包/作物系统协作。
    // - enableToolOnSpace / enableToolOnLeftClick 控制是否用空格/左键触发当前工具。
    // - onSpacePressed / onKeyPressedHook / onMouseDownHook 提供场景层自定义扩展。
    void registerCommonInputHandlers(
        cocos2d::Node* ownerNode,
        UIController* ui,
        std::shared_ptr<Game::Inventory> inventory,
        CropSystem* cropSystem,
        bool enableToolOnSpace,
        bool enableToolOnLeftClick,
        std::function<void()> onSpacePressed,
        std::function<void(cocos2d::EventKeyboard::KeyCode)> onKeyPressedHook,
        std::function<void(cocos2d::EventMouse*)> onMouseDownHook);

    // 每帧更新：根据当前输入状态推进位置/动画/冲刺状态，并驱动相机与光标跟随。
    void update(float dt);
    // 锁定/解锁玩家移动（例如对话或 UI 打开时禁止移动）。
    void setMovementLocked(bool locked) { _movementLocked = locked; }
    // 查询当前是否禁止移动。
    bool isMovementLocked() const { return _movementLocked; }

    // 设置移动速度倍率（<=0 时重置为 1.0）。
    void setMoveSpeedMultiplier(float m) { _moveSpeedMultiplier = (m <= 0.0f ? 1.0f : m); }
    // 获取当前移动速度倍率。
    float moveSpeedMultiplier() const { return _moveSpeedMultiplier; }

    // 获取玩家最近一次移动方向，用于工具/交互的朝向判定。
    cocos2d::Vec2 lastDir() const { return _lastDir; }

private:
    Game::IPlayerView* _player = nullptr;
    IMapController* _map = nullptr;
    cocos2d::Node* _worldNode = nullptr;

    cocos2d::Node* _inputOwner = nullptr;
    UIController* _ui = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    CropSystem* _cropSystem = nullptr;
    bool _enableToolOnSpace = false;
    bool _enableToolOnLeftClick = false;
    std::function<void()> _onSpacePressed;
    std::function<void(cocos2d::EventKeyboard::KeyCode)> _onKeyPressedHook;
    std::function<void(cocos2d::EventMouse*)> _onMouseDownHook;
    cocos2d::EventListenerKeyboard* _kbListener = nullptr;
    cocos2d::EventListenerMouse* _mouseListener = nullptr;
    cocos2d::EventListenerTouchOneByOne* _touchListener = nullptr;

    // input state
    bool _up = false, _down = false, _left = false, _right = false;
    float _baseSpeed = 140.0f;
    float _sprintSpeed = 240.0f;
    float _moveHeldDuration = 0.0f;
    bool  _isSprinting = false;
    float _sprintThreshold = 0.5f;
    float _moveSpeedMultiplier = 1.0f;
    cocos2d::Vec2 _lastDir = cocos2d::Vec2(0, -1);
    bool _movementLocked = false;
    int _toolRangeModifierHeld = 0;
};

}
// namespace Controllers
