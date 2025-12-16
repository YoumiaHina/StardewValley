#include "Game/Tree.h"

using namespace cocos2d;

namespace Game {

Tree* Tree::create(const std::string& texture) {
    Tree* t = new (std::nothrow) Tree();
    if (t && t->initWithTexture(texture)) { t->autorelease(); return t; }
    CC_SAFE_DELETE(t); return nullptr;
}

bool Tree::initWithTexture(const std::string& texture) {
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

void Tree::applyDamage(int amount) {
    _hp = std::max(0, _hp - std::max(0, amount));
}

Rect Tree::footRect() const {
    float w = 12.0f, h = 12.0f;
    Vec2 base = getPosition();
    return Rect(base.x - w * 0.5f, base.y + 4.0f, w, h);
}

void Tree::playDestructionAnimation(const std::function<void()>& onComplete) {

    if (_falling || !_sprite) { if (onComplete) onComplete(); return; }
    _falling = true;
    float angle = (RandomHelper::random_int(0, 1) == 0) ? -90.0f : 90.0f;
    auto rotate = EaseCubicActionOut::create(RotateBy::create(0.45f, angle));
    auto fade = FadeOut::create(0.25f);
    auto seq = Sequence::create(rotate, fade, CallFunc::create([this, onComplete] { if (onComplete) onComplete(); }), nullptr);
    _sprite->runAction(seq);
}

}
