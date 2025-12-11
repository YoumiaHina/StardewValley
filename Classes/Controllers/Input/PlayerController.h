/**
 * PlayerController: 管理输入、移动、冲刺与动画，独立于场景。
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include "Controllers/IMapController.h"

namespace Game { class IPlayerView; }

namespace Controllers {

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

    void update(float dt);
    void setMovementLocked(bool locked) { _movementLocked = locked; }
    bool isMovementLocked() const { return _movementLocked; }

    cocos2d::Vec2 lastDir() const { return _lastDir; }

private:
    Game::IPlayerView* _player = nullptr;
    IMapController* _map = nullptr;
    cocos2d::Node* _worldNode = nullptr;

    // input state
    bool _up = false, _down = false, _left = false, _right = false;
    float _baseSpeed = 140.0f;
    float _sprintSpeed = 240.0f;
    float _moveHeldDuration = 0.0f;
    bool  _isSprinting = false;
    float _sprintThreshold = 0.5f;
    cocos2d::Vec2 _lastDir = cocos2d::Vec2(0, -1);
    bool _movementLocked = false;
};

}
// namespace Controllers
