#include "RainLayer.h"
#include <algorithm>

namespace Controllers {

RainLayer* RainLayer::create(const cocos2d::Size& area) {
    auto p = new (std::nothrow) RainLayer();
    if (!p) return nullptr;
    if (!p->init()) {
        delete p;
        return nullptr;
    }
    p->autorelease();
    p->setArea(area);
    p->ensureDraw();
    p->initDrops();
    p->setActive(false);
    return p;
}

void RainLayer::setArea(const cocos2d::Size& area) {
    _area = area;
    if (_area.width < 0) _area.width = 0;
    if (_area.height < 0) _area.height = 0;
}

void RainLayer::setActive(bool active) {
    if (active) {
        setVisible(true);
        if (!isScheduled(CC_SCHEDULE_SELECTOR(RainLayer::tick))) {
            schedule(CC_SCHEDULE_SELECTOR(RainLayer::tick));
        }
    } else {
        setVisible(false);
        unschedule(CC_SCHEDULE_SELECTOR(RainLayer::tick));
        if (_draw) _draw->clear();
    }
}

void RainLayer::ensureDraw() {
    if (_draw) return;
    _draw = cocos2d::DrawNode::create();
    if (!_draw) return;
    addChild(_draw, 0);
}

void RainLayer::initDrops() {
    std::uniform_real_distribution<float> rx(0.0f, std::max(1.0f, _area.width));
    std::uniform_real_distribution<float> ry(0.0f, std::max(1.0f, _area.height));
    std::uniform_real_distribution<float> rs(360.0f, 820.0f);
    std::uniform_real_distribution<float> rl(6.0f, 14.0f);

    const int count = 140;
    _drops.clear();
    _drops.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        Drop d;
        d.x = rx(_rng);
        d.y = ry(_rng);
        d.speed = rs(_rng);
        d.len = rl(_rng);
        _drops.push_back(d);
    }
}

void RainLayer::tick(float dt) {
    if (!_draw) return;
    if (_area.width <= 0.0f || _area.height <= 0.0f) return;
    if (_drops.empty()) initDrops();

    std::uniform_real_distribution<float> rx(0.0f, std::max(1.0f, _area.width));
    std::uniform_real_distribution<float> rSpeed(360.0f, 820.0f);
    std::uniform_real_distribution<float> rLen(6.0f, 14.0f);

    const float windX = 120.0f;
    _draw->clear();
    cocos2d::Color4F col(0.75f, 0.80f, 1.00f, 0.35f);

    for (auto& d : _drops) {
        d.x += windX * dt;
        d.y -= d.speed * dt;
        if (d.x > _area.width + 40.0f) d.x -= (_area.width + 80.0f);
        if (d.y < -40.0f) {
            d.x = rx(_rng);
            d.y = _area.height + 40.0f;
            d.speed = rSpeed(_rng);
            d.len = rLen(_rng);
        }

        cocos2d::Vec2 a(d.x, d.y);
        cocos2d::Vec2 b(d.x - d.len * 0.35f, d.y + d.len);
        _draw->drawLine(a, b, col);
    }
}

} // namespace Controllers

