#include "Game/Weed.h"
#include <algorithm>

using namespace cocos2d;

namespace Game {

Weed* Weed::create(const std::string& texture) {
    Weed* w = new (std::nothrow) Weed();
    if (w && w->initWithTexture(texture)) { w->autorelease(); return w; }
    CC_SAFE_DELETE(w);
    return nullptr;
}

bool Weed::initWithTexture(const std::string& texture) {
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

void Weed::setBrokenTexture(const std::string& texture) {
    _brokenTexture = texture;
}

void Weed::applyDamage(int amount) {
    _hp = std::max(0, _hp - std::max(0, amount));
}

Rect Weed::footRect() const {
    float w = 12.0f;
    float h = 12.0f;
    Vec2 base = getPosition();
    return Rect(base.x - w * 0.5f, base.y + h * 0.3f, w, h);
}

void Weed::playDestructionAnimation(const std::function<void()>& onComplete) {
    if (_breaking || !_sprite) {
        if (onComplete) onComplete();
        return;
    }
    _breaking = true;
    std::string tex = _brokenTexture.empty() ? std::string("FarmEnvironment/grass_broken.png") : _brokenTexture;
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

