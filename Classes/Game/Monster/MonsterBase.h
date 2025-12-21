#pragma once

#include <vector>
#include <functional>
#include "Game/Monster.h"
#include "Game/Item.h"

namespace cocos2d {
class Sprite;
}

namespace Game {

class MonsterBase {
public:
    virtual ~MonsterBase() = default;

    virtual Monster::Type monsterType() const = 0;

    MonsterDef def_;
    std::vector<ItemType> drops_;

    virtual void initInstance(Monster& m) const;
    virtual std::vector<ItemType> dropsFor(const Monster& m) const;
    virtual void playStaticAnimation(const Monster& m, cocos2d::Sprite* sprite) const = 0;
    virtual void playMoveAnimation(const Monster& m, cocos2d::Sprite* sprite) const = 0;
    virtual void playDeathAnimation(const Monster& m, cocos2d::Sprite* sprite, const std::function<void()>& onComplete) const = 0;
};

const MonsterBase& monsterInfoFor(Monster::Type type);

} // namespace Game
