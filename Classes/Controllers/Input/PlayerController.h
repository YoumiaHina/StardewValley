/**
 * PlayerController: 管理输入、移动、冲刺与动画，独立于场景。
 */
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

class PlayerController {
public:
    PlayerController(Game::IPlayerView* player,
                     IMapController* map,
                     cocos2d::Node* worldNode,
                     float baseSpeed = 140.0f,
                     float sprintSpeed = 240.0f,
                     float sprintThreshold = 0.5f)
    : _player(player), _map(map), _worldNode(worldNode),
      _baseSpeed(baseSpeed), _sprintSpeed(sprintSpeed), _sprintThreshold(sprintThreshold) {}

    void onKeyPressed(cocos2d::EventKeyboard::KeyCode code);
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode code);

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

    void update(float dt);
    void setMovementLocked(bool locked) { _movementLocked = locked; }
    bool isMovementLocked() const { return _movementLocked; }

    void setMoveSpeedMultiplier(float m) { _moveSpeedMultiplier = (m <= 0.0f ? 1.0f : m); }
    float moveSpeedMultiplier() const { return _moveSpeedMultiplier; }

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
