/**
 * GameScene: Main game scene placeholder.
 */
#pragma once

#include "cocos2d.h"

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(GameScene);

    void update(float dt) override;

private:
    cocos2d::DrawNode* _player = nullptr;
    // input state
    bool _up = false;
    bool _down = false;
    bool _left = false;
    bool _right = false;
    bool _wKeyPressed = false; // specifically track W key for sprint
    // movement params
    float _baseSpeed = 140.0f;      // pixels per second
    float _sprintSpeed = 240.0f;    // pixels per second when sprinting
    float _wHeldDuration = 0.0f;
    bool  _isSprinting = false;
    const float _sprintThreshold = 0.5f; // seconds to hold W to sprint
};