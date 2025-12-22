#pragma once

#include "cocos2d.h"
#include <string>
#include <vector>
#include <functional>
#include "Game/Item.h"

namespace Game {

enum class MonsterType { GreenSlime, BlueSlime, RedSlime, Bug, Ghost };

struct MonsterDef {
    int hp = 0;
    int dmg = 0;
    int def = 0;
    int searchRangeTiles = 0;
    float moveSpeed = 0.0f;
    bool isCollisionAffected = true;
    const char* name = nullptr;
};

class MonsterBase {
public:
    virtual ~MonsterBase() = default;

    virtual MonsterType monsterType() const = 0;

    MonsterDef def_;
    std::vector<ItemType> drops_;

    virtual void playStaticAnimation(cocos2d::Sprite* sprite) const = 0;
    virtual void playMoveAnimation(const cocos2d::Vec2& velocity, cocos2d::Sprite* sprite) const = 0;
    virtual void playDeathAnimation(cocos2d::Sprite* sprite, const std::function<void()>& onComplete) const = 0;
};

const MonsterBase& monsterInfoFor(MonsterType type);

} // namespace Game
