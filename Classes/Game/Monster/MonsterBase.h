#pragma once

#include "cocos2d.h"
#include <string>
#include <vector>
#include <functional>
#include "Game/Item.h"

namespace Game {

struct Monster {
    enum class Type { GreenSlime, BlueSlime, RedSlime, Bug, Ghost };
    Type type;
    cocos2d::Vec2 pos;
    int hp = 0;
    int maxHp = 0;
    int dmg = 0;
    int def = 0;
    int searchRangeTiles = 0;
    bool elite = false;
    float moveSpeed = 0.0f;
    bool isCollisionAffected = true;
    cocos2d::Vec2 velocity;
    int textureVariant = 0;
    float attackCooldown = 0.0f;
    cocos2d::Sprite* sprite = nullptr;
    std::string name;

    std::vector<ItemType> getDrops() const;
};

struct MonsterDef {
    int hp = 0;
    int dmg = 0;
    int def = 0;
    int searchRangeTiles = 0;
    float moveSpeed = 0.0f;
    bool isCollisionAffected = true;
    const char* name = nullptr;
};

class MonsterDefs {
public:
    static const MonsterDef& def(Monster::Type type);
    static void initMonster(Monster& m);
    static std::vector<ItemType> dropsFor(const Monster& m);
};

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
Monster makeMonsterForType(Monster::Type type);
std::string monsterTexturePath(const Monster& m);

} // namespace Game
