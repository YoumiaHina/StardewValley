#pragma once

#include "cocos2d.h"

namespace Game {

enum class MonsterType { GreenSlime, BlueSlime, RedSlime, Bug, Ghost };

struct Monster {
    MonsterType type = MonsterType::GreenSlime;
    cocos2d::Vec2 pos;
    cocos2d::Vec2 velocity;
    int hp = 0;
    int dmg = 0;
    int def = 0;
    int searchRangeTiles = 0;
    float moveSpeed = 0.0f;
    float attackCooldown = 0.0f;
    bool isCollisionAffected = true;
    const char* name = nullptr;
    cocos2d::Sprite* sprite = nullptr;
};

} // namespace Game
