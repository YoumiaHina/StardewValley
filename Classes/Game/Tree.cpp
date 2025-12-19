#include "Game/Tree.h"
#include <algorithm>

using namespace cocos2d;

namespace Game {

namespace {
    const char* seasonName(int seasonIndex) {
        switch ((seasonIndex % 4 + 4) % 4) {
            case 0: return "spring";
            case 1: return "summer";
            case 2: return "fall";
            case 3: return "winter";
        }
        return "spring";
    }
}

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

void Tree::setKind(TreeKind kind) {
    _kind = kind;
    if (_sprite) {
        _sprite->setTexture(texturePath(_kind, _seasonIndex));
        if (_sprite->getTexture()) {
            _sprite->getTexture()->setAliasTexParameters();
        }
    }
}

TreeKind Tree::kind() const { return _kind; }

void Tree::setSeasonIndex(int seasonIndex) {
    int normalized = (seasonIndex % 4 + 4) % 4;
    if (_seasonIndex == normalized) return;
    _seasonIndex = normalized;
    if (_sprite) {
        _sprite->setTexture(texturePath(_kind, _seasonIndex));
        if (_sprite->getTexture()) {
            _sprite->getTexture()->setAliasTexParameters();
        }
    }
}

int Tree::seasonIndex() const { return _seasonIndex; }

std::string Tree::texturePath(TreeKind kind, int seasonIndex) {
    const char* sn = seasonName(seasonIndex);
    const char* base = (kind == TreeKind::Tree2) ? "FarmEnvironment/tree2_" : "FarmEnvironment/tree1_";
    return std::string(base) + sn + ".png";
}

void Tree::applyDamage(int amount) {
    _hp = std::max(0, _hp - std::max(0, amount));
}

Rect Tree::footRect() const {
    float w = 12.0f, h = 12.0f;
    Vec2 base = getPosition();
    return Rect(base.x - w * 0.5f, base.y + h * 0.3f, w, h);
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
