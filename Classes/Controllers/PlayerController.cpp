#include "Controllers/PlayerController.h"
#include "Game/PlayerAppearance.h"
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
            _player->setDirection(dx > 0 ? Game::PlayerAppearance::Direction::RIGHT
                                          : Game::PlayerAppearance::Direction::LEFT);
        } else {
            _player->setDirection(dy > 0 ? Game::PlayerAppearance::Direction::UP
                                          : Game::PlayerAppearance::Direction::DOWN);
        }
    }
    _player->updateAnimation(dt);

    float speed = _isSprinting ? _sprintSpeed : _baseSpeed;
    Vec2 delta = dir * speed * dt;
    Vec2 next = _player->getPosition() + delta;

    // Map-specific clamp and collision
    Vec2 clamped = _map->clampPosition(_player->getPosition(), next, /*radius*/12.0f);
    _player->setPosition(clamped);

    // Cursor (Farm) and camera follow
    _map->updateCursor(_player->getPosition(), _lastDir);

    if (_worldNode) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();
        Vec2 screenPos = _worldNode->convertToWorldSpace(_player->getPosition());
        float marginX = visibleSize.width * 0.25f;
        float marginY = visibleSize.height * 0.25f;
        float left = origin.x + marginX;
        float right = origin.x + visibleSize.width - marginX;
        float bottom = origin.y + marginY;
        float top = origin.y + visibleSize.height - marginY;
        Vec2 cam = _worldNode->getPosition();
        if (screenPos.x < left)   cam.x += left - screenPos.x;
        if (screenPos.x > right)  cam.x += right - screenPos.x;
        if (screenPos.y < bottom) cam.y += bottom - screenPos.y;
        if (screenPos.y > top)    cam.y += top - screenPos.y;

        float scale = _worldNode->getScale();
        cocos2d::Size mapSize = _map->getContentSize();
        float mapW = mapSize.width;
        float mapH = mapSize.height;
        auto org = _map->getOrigin();

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
}

} // namespace Controllers