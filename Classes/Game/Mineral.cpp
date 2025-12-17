#include "Game/Mineral.h"
#include "Game/GameConfig.h"
#include <algorithm>

using namespace cocos2d;

namespace Game {

Mineral* Mineral::create(const std::string& texture) {
    Mineral* r = new (std::nothrow) Mineral();
    if (r && r->initWithTexture(texture)) { r->autorelease(); return r; }
    CC_SAFE_DELETE(r);
    return nullptr;
}

bool Mineral::initWithTexture(const std::string& texture) {
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

void Mineral::setBrokenTexture(const std::string& texture) {
    _brokenTexture = texture;
}

void Mineral::applyDamage(int amount) {
    _hp = std::max(0, _hp - std::max(0, amount));
}

Rect Mineral::footRect() const {
    float w = 12.0f;
    float h = 12.0f;
    Vec2 base = getPosition();
    if (_type == MineralType::HugeRock) {
        w = 24.0f;
        h = 20.0f;
    }
    return Rect(base.x - w * 0.5f, base.y - h * 0.4f, w, h);
}

Size Mineral::spriteContentSize() const {
    return _sprite ? _sprite->getContentSize() : Size::ZERO;
}

void Mineral::playDestructionAnimation(const std::function<void()>& onComplete) {
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

ItemType mineralDropItem(MineralType type) {
    switch (type) {
        case MineralType::CopperOre: return ItemType::CopperGrain;
        case MineralType::IronOre:   return ItemType::IronGrain;
        case MineralType::GoldOre:   return ItemType::GoldGrain;
        default:                     return ItemType::Stone;
    }
}

}
