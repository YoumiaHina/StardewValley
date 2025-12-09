#pragma once

#include "cocos2d.h"

namespace Game {

class Tree : public cocos2d::Node {
public:
    static Tree* create(const std::string& texture);
    bool initWithTexture(const std::string& texture);

    void applyDamage(int amount);
    int hp() const { return _hp; }
    bool dead() const { return _hp <= 0; }

    cocos2d::Rect footRect() const;
    void playFallAnimation(const std::function<void()>& onComplete);

private:
    cocos2d::Sprite* _sprite = nullptr;
    int _hp = 3;
    bool _falling = false;
};

}
