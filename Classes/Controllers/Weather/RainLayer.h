#pragma once

#include "cocos2d.h"
#include <vector>
#include <random>

namespace Controllers {

class RainLayer : public cocos2d::Node {
public:
    static RainLayer* create(const cocos2d::Size& area);

    void setArea(const cocos2d::Size& area);
    void setActive(bool active);

private:
    struct Drop {
        float x = 0.0f;
        float y = 0.0f;
        float speed = 0.0f;
        float len = 0.0f;
    };

    void ensureDraw();
    void initDrops();
    void tick(float dt);

private:
    cocos2d::DrawNode* _draw = nullptr;
    cocos2d::Size _area;
    std::vector<Drop> _drops;
    std::mt19937 _rng{ std::random_device{}() };
};

} // namespace Game

