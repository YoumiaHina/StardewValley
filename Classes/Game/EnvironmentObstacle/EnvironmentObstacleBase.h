#pragma once

#include "cocos2d.h"
#include <functional>

namespace Game {

class EnvironmentObstacleBase : public cocos2d::Node {
public:
    virtual ~EnvironmentObstacleBase() = default;

    virtual void applyDamage(int amount) = 0;
    virtual int hp() const = 0;
    virtual bool dead() const = 0;

    virtual cocos2d::Rect footRect() const = 0;
    virtual void playDestructionAnimation(const std::function<void()>& onComplete) = 0;
};

}

