#pragma once

#include "cocos2d.h"
#include <functional>
#include "Game/EnvironmentObstacleBase.h"

namespace Game {

class Rock : public EnvironmentObstacleBase {
public:
    static Rock* create(const std::string& texture);
    bool initWithTexture(const std::string& texture);

    void applyDamage(int amount) override;
    int hp() const override { return _hp; }
    bool dead() const override { return _hp <= 0; }

    cocos2d::Rect footRect() const override;
    void playBreakAnimation(const std::function<void()>& onComplete);
    void playDestructionAnimation(const std::function<void()>& onComplete) override;

private:
    cocos2d::Sprite* _sprite = nullptr;
    int _hp = 1;
    bool _breaking = false;
};

struct RockPos {
    int c = 0;
    int r = 0;
};

}
