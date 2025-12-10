#include "Controllers/PlayerController.h"
#include "Controllers/PlayerActionController.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Controllers {

void PlayerController::onKeyPressed(EventKeyboard::KeyCode code) {
    switch (code) {
        case EventKeyboard::KeyCode::KEY_W:
        case EventKeyboard::KeyCode::KEY_UP_ARROW:    _up = true; break;
        case EventKeyboard::KeyCode::KEY_S:
        case EventKeyboard::KeyCode::KEY_DOWN_ARROW:  _down = true; break;
        case EventKeyboard::KeyCode::KEY_A:
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:  _left = true; break;
        case EventKeyboard::KeyCode::KEY_D:
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: _right = true; break;
        default: break;
    }
}

void PlayerController::onKeyReleased(EventKeyboard::KeyCode code) {
    switch (code) {
        case EventKeyboard::KeyCode::KEY_W:
        case EventKeyboard::KeyCode::KEY_UP_ARROW:    _up = false; break;
        case EventKeyboard::KeyCode::KEY_S:
        case EventKeyboard::KeyCode::KEY_DOWN_ARROW:  _down = false; break;
        case EventKeyboard::KeyCode::KEY_A:
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:  _left = false; break;
        case EventKeyboard::KeyCode::KEY_D:
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: _right = false; break;
        default: break;
    }
}

void PlayerController::update(float dt) {
    if (!_player || !_map) return;

    // sprint timing: holding ANY movement key
    bool movementHeld = (_up || _down || _left || _right);
    if (movementHeld) {
        _moveHeldDuration += dt;
        _isSprinting = (_moveHeldDuration >= _sprintThreshold);
    } else {
        _moveHeldDuration = 0.0f;
        _isSprinting = false;
    }

    float dx = 0.0f, dy = 0.0f;
    if (_left)  dx -= 1.0f;
    if (_right) dx += 1.0f;
    if (_down)  dy -= 1.0f;
    if (_up)    dy += 1.0f;

    Vec2 dir(dx, dy);
    if (dir.lengthSquared() > 0.0f) {
        dir.normalize();
        _lastDir = dir;
    }

    bool isMoving = (dx != 0 || dy != 0);
    _player->setMoving(isMoving);
    if (isMoving) {
        if (std::abs(dx) > std::abs(dy)) {
            _player->setDirection(dx > 0 ? Controllers::PlayerActionController::Direction::RIGHT
                                          : Controllers::PlayerActionController::Direction::LEFT);
        } else {
            _player->setDirection(dy > 0 ? Controllers::PlayerActionController::Direction::UP
                                          : Controllers::PlayerActionController::Direction::DOWN);
        }
    }
    _player->updateAnimation(dt);

    float speed = _isSprinting ? _sprintSpeed : _baseSpeed;
    Vec2 delta = dir * speed * dt;
    Vec2 next = _player->getPosition() + delta;

    Vec2 clamped = _map->clampPosition(_player->getPosition(), next, /*radius*/8.0f);
    _player->setPosition(clamped);

    // Cursor (Farm) and camera follow
    _map->updateCursor(_player->getPosition(), _lastDir);

    if (_worldNode) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        float scale = _worldNode->getScale();
        auto org = _map->getOrigin();
        auto playerPos = _player->getPosition();
        Vec2 screenCenter(origin.x + visibleSize.width * 0.5f, origin.y + visibleSize.height * 0.5f);
        Vec2 cam = screenCenter - (org + playerPos) * scale;

        cocos2d::Size mapSize = _map->getContentSize();
        float mapW = mapSize.width;
        float mapH = mapSize.height;

        float minX = (origin.x + visibleSize.width) - (org.x + mapW) * scale;
        float maxX = origin.x - org.x * scale;
        float minY = (origin.y + visibleSize.height) - (org.y + mapH) * scale;
        float maxY = origin.y - org.y * scale;

        if (mapW * scale <= visibleSize.width) {
            cam.x = (origin.x + visibleSize.width * 0.5f) - (org.x + mapW * 0.5f) * scale;
        } else {
            cam.x = std::max(minX, std::min(maxX, cam.x));
        }

        if (mapH * scale <= visibleSize.height) {
            cam.y = (origin.y + visibleSize.height * 0.5f) - (org.y + mapH * 0.5f) * scale;
        } else {
            cam.y = std::max(minY, std::min(maxY, cam.y));
        }
        _worldNode->setPosition(cam);
    }

    if (_map) { _map->sortActorWithEnvironment(_player); }
}

} // namespace Controllers
