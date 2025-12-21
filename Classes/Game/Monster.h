#pragma once

#include "cocos2d.h"
#include <string>
#include <vector>
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

int slimeVariantForFloor(int floor);
std::string slimeTexturePathForVariant(int variant);
Monster makeMonsterForType(Monster::Type type);
std::string monsterTexturePath(const Monster& m);

} // namespace Game
