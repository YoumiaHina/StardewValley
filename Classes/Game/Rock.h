#pragma once

#include "cocos2d.h"
#include <functional>

namespace Game {

class Rock : public cocos2d::Node {
public:
    static Rock* create(const std::string& texture);
    bool initWithTexture(const std::string& texture);

    void applyDamage(int amount);
    int hp() const { return _hp; }
    bool dead() const { return _hp <= 0; }

    cocos2d::Rect footRect() const;
    void playBreakAnimation(const std::function<void()>& onComplete);

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

