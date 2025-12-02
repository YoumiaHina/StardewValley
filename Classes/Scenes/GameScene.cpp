/**
 * GameScene: Main game scene placeholder.
 */
#include "Scenes/GameScene.h"
#include "cocos2d.h"

USING_NS_CC;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // Placeholder label
    auto label = Label::createWithTTF("Game Scene Placeholder", "fonts/Marker Felt.ttf", 30);
    if (label) {
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height * 0.85f));
        this->addChild(label, 1);
    }

    // Simple placeholder player: a colored square
    _player = DrawNode::create();
    const float size = 16.f;
    Vec2 verts[4] = { Vec2(-size, -size), Vec2(size, -size), Vec2(size, size), Vec2(-size, size) };
    _player->drawSolidPoly(verts, 4, Color4F(0.2f, 0.7f, 0.9f, 1.0f));
    _player->setPosition(Vec2(origin.x + visibleSize.width / 2,
                              origin.y + visibleSize.height / 2));
    this->addChild(_player, 0);

    // Keyboard movement (placeholder)
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*) {
        switch (code) {
            case EventKeyboard::KeyCode::KEY_W:          _up = true; _wKeyPressed = true; break;
            case EventKeyboard::KeyCode::KEY_UP_ARROW:   _up = true; break;
            case EventKeyboard::KeyCode::KEY_S:          _down = true; break;
            case EventKeyboard::KeyCode::KEY_DOWN_ARROW: _down = true; break;
            case EventKeyboard::KeyCode::KEY_A:          _left = true; break;
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW: _left = true; break;
            case EventKeyboard::KeyCode::KEY_D:          _right = true; break;
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:_right = true; break;
            default: break;
        }
    };
    listener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) {
        switch (code) {
            case EventKeyboard::KeyCode::KEY_W:          _up = false; _wKeyPressed = false; _wHeldDuration = 0.0f; _isSprinting = false; break;
            case EventKeyboard::KeyCode::KEY_UP_ARROW:   _up = false; break;
            case EventKeyboard::KeyCode::KEY_S:          _down = false; break;
            case EventKeyboard::KeyCode::KEY_DOWN_ARROW: _down = false; break;
            case EventKeyboard::KeyCode::KEY_A:          _left = false; break;
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW: _left = false; break;
            case EventKeyboard::KeyCode::KEY_D:          _right = false; break;
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:_right = false; break;
            default: break;
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    this->scheduleUpdate();

    return true;
}

void GameScene::update(float dt) {
    // sprint timing based on holding W only
    if (_wKeyPressed) {
        _wHeldDuration += dt;
        _isSprinting = (_wHeldDuration >= _sprintThreshold);
    } else {
        _isSprinting = false;
    }

    // accumulate direction from pressed keys for diagonal movement
    float dx = 0.0f;
    float dy = 0.0f;
    if (_left)  dx -= 1.0f;
    if (_right) dx += 1.0f;
    if (_down)  dy -= 1.0f;
    if (_up)    dy += 1.0f;

    if (dx == 0.0f && dy == 0.0f) return;

    cocos2d::Vec2 dir(dx, dy);
    dir.normalize(); // keep consistent speed when moving diagonally

    float speed = _isSprinting ? _sprintSpeed : _baseSpeed;
    cocos2d::Vec2 delta = dir * speed * dt;
    _player->setPosition(_player->getPosition() + delta);
}