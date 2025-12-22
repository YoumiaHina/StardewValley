#include "Game/EnvironmentObstacle/Rock.h"
#include <algorithm>

using namespace cocos2d;

namespace Game {

std::string Rock::texturePath(RockKind kind) {
    return (kind == RockKind::Rock2) ? std::string("FarmEnvironment/rock2.png")
                                    : std::string("FarmEnvironment/rock1.png");
}

Rock* Rock::create(const std::string& texture) {
    Rock* r = new (std::nothrow) Rock();
    if (r && r->initWithTexture(texture)) { r->autorelease(); return r; }
    CC_SAFE_DELETE(r);
    return nullptr;
}

bool Rock::initWithTexture(const std::string& texture) {
    if (!Node::init()) return false;
    _sprite = Sprite::create(texture);
    if (_sprite) {
        addChild(_sprite);
        _sprite->setAnchorPoint(Vec2(0.5f, 0.0f));
        _sprite->setPosition(Vec2(0, 0));
        if (_sprite->getTexture()) {
            _sprite->getTexture()->setAliasTexParameters();
        }
    }
    return true;
}

void Rock::setBrokenTexture(const std::string& texture) {
    _brokenTexture = texture;
}

void Rock::setKind(RockKind kind) {
    _kind = kind;
    if (_sprite) {
        _sprite->setTexture(texturePath(_kind));
        if (_sprite->getTexture()) {
            _sprite->getTexture()->setAliasTexParameters();
        }
    }
}

RockKind Rock::kind() const { return _kind; }

void Rock::applyDamage(int amount) {
    _hp = std::max(0, _hp - std::max(0, amount));
}

Rect Rock::footRect() const {
    float w = 12.0f;
    float h = 12.0f;
    Vec2 base = getPosition();
    return Rect(base.x - w * 0.5f, base.y + h * 0.3f, w, h);
}

void Rock::playDestructionAnimation(const std::function<void()>& onComplete) {
    if (_breaking || !_sprite) {
        if (onComplete) onComplete();
        return;
    }
    _breaking = true;
    std::string tex = _brokenTexture.empty() ? std::string("FarmEnvironment/rock_broken.png") : _brokenTexture;
    _sprite->setTexture(tex);
    if (_sprite->getTexture()) {
        _sprite->getTexture()->setAliasTexParameters();
    }
    auto scale = EaseCubicActionOut::create(ScaleBy::create(0.25f, 1.2f));
    auto fade = FadeOut::create(0.25f);
    auto seq = Sequence::create(scale, fade, CallFunc::create([this, onComplete] {
        if (onComplete) onComplete();
        }), nullptr);
    _sprite->runAction(seq);
}

}
