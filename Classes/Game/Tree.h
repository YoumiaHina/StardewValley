#pragma once

#include "cocos2d.h"
#include "Game/EnvironmentObstacleBase.h"

namespace Game {

class Tree : public EnvironmentObstacleBase {
public:
    static Tree* create(const std::string& texture);
    bool initWithTexture(const std::string& texture);

    void applyDamage(int amount) override;
    int hp() const override { return _hp; }
    bool dead() const override { return _hp <= 0; }

    cocos2d::Rect footRect() const override;
    void playDestructionAnimation(const std::function<void()>& onComplete) override;

private:
    cocos2d::Sprite* _sprite = nullptr;
    int _hp = 3;
    bool _falling = false;
};

struct TreePos {
    int c = 0;
    int r = 0;
};

}
