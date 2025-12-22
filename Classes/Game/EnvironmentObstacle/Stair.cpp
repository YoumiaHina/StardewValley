#include "Game/EnvironmentObstacle/Stair.h"

using namespace cocos2d;

namespace Game {

Stair* Stair::create(const std::string& texture) {
    Stair* s = new (std::nothrow) Stair();
    if (s && s->initWithTexture(texture)) { s->autorelease(); return s; }
    CC_SAFE_DELETE(s);
    return nullptr;
}

bool Stair::initWithTexture(const std::string& texture) {
    if (!Node::init()) return false;
    _sprite = Sprite::create(texture);
    if (_sprite) {
        addChild(_sprite);
        _sprite->setAnchorPoint(Vec2(0.5f, 0.5f));
        _sprite->setPosition(Vec2(0, 0));
        if (_sprite->getTexture()) {
            _sprite->getTexture()->setAliasTexParameters();
        }
    }
    return true;
}

void Stair::applyDamage(int) {
}

Rect Stair::footRect() const {
    float w = 12.0f;
    float h = 12.0f;
    Vec2 base = getPosition();
    return Rect(base.x - w * 0.5f, base.y, w, h);
}

Size Stair::spriteContentSize() const {
    if (_sprite) {
        return _sprite->getContentSize();
    }
    return Size::ZERO;
}

void Stair::playDestructionAnimation(const std::function<void()>& onComplete) {
    if (_removing) {
        if (onComplete) onComplete();
        return;
    }
    _removing = true;
    if (_sprite && _sprite->getParent()) {
        _sprite->removeFromParent();
        _sprite = nullptr;
    }
    if (onComplete) onComplete();
}

}
