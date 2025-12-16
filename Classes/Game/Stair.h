#pragma once

#include "cocos2d.h"
#include <functional>
#include "Game/EnvironmentObstacleBase.h"

namespace Game {

class Stair : public EnvironmentObstacleBase {
public:
    static Stair* create(const std::string& texture);
    bool initWithTexture(const std::string& texture);

    void applyDamage(int amount) override;
    int hp() const override { return _hp; }
    bool dead() const override { return _hp <= 0; }

    cocos2d::Rect footRect() const override;
    cocos2d::Size spriteContentSize() const;
    void playDestructionAnimation(const std::function<void()>& onComplete) override;

private:
    cocos2d::Sprite* _sprite = nullptr;
    int _hp = 1;
    bool _removing = false;
};

}
