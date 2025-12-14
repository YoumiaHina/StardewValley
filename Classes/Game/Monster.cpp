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
            m.hp = 24;
            m.maxHp = 24;
            m.dmg = 5;
            m.def = 0;
            m.searchRangeTiles = 4;
            m.moveSpeed = 40.0f;
            m.name = "Green Slime";
            break;
    }
    return m;
}

} // namespace Game
