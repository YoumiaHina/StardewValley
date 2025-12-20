#include "Game/Monster.h"

namespace Game {

int slimeVariantForFloor(int floor) {
    if (floor >= 80) return 2;
    if (floor >= 40) return 1;
    return 0;
}

std::string slimeTexturePathForVariant(int variant) {
    if (variant == 1) return "Monster/GreenSlimeVariation1Static.png";
    if (variant == 2) return "Monster/GreenSlimeVariation2Static.png";
    return "Monster/GreenSlimeStatic.png";
}

Monster makeMonsterForType(Monster::Type type) {
    Monster m{};
    m.type = type;
    switch (type) {
        case Monster::Type::RockSlime:
            m.isCollisionAffected = true;
            m.hp = 24;
            m.maxHp = 24;
            m.dmg = 5;
            m.def = 0;
            m.searchRangeTiles = 4;
            m.moveSpeed = 40.0f;
            m.name = "Green Slime";
            break;
        case Monster::Type::Bug:
            m.isCollisionAffected = false;
            m.hp = 16;
            m.maxHp = 16;
            m.dmg = 3;
            m.def = 0;
            m.searchRangeTiles = 6;
            m.moveSpeed = 100.0f;
            m.name = "Bug";
            break;
        case Monster::Type::Ghost:
            m.isCollisionAffected = false;
            m.hp = 80;
            m.maxHp = 80;
            m.dmg = 15;
            m.def = 2;
            m.searchRangeTiles = 20;
            m.moveSpeed = 400.0f;
            m.name = "Ghost";
            break;
    }
    return m;
}

std::string monsterTexturePath(const Monster& m) {
    switch (m.type) {
        case Monster::Type::RockSlime:
            return slimeTexturePathForVariant(m.textureVariant);
        case Monster::Type::Bug:
            return "Monster/Bug.png";
        case Monster::Type::Ghost:
            return "Monster/Ghost.png";
    }
    return slimeTexturePathForVariant(m.textureVariant);
}

} // namespace Game
