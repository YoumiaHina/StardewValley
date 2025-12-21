#include "Game/Monster.h"
#include "Game/Monster/MonsterBase.h"

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

const MonsterDef& MonsterDefs::def(Monster::Type type) {
    return monsterInfoFor(type).def_;
}

void MonsterDefs::initMonster(Monster& m) {
    const auto& info = monsterInfoFor(m.type);
    info.initInstance(m);
}

std::vector<ItemType> MonsterDefs::dropsFor(const Monster& m) {
    const auto& info = monsterInfoFor(m.type);
    return info.dropsFor(m);
}

Monster makeMonsterForType(Monster::Type type) {
    Monster m{};
    m.type = type;
    MonsterDefs::initMonster(m);
    return m;
}

std::vector<ItemType> Monster::getDrops() const {
    return MonsterDefs::dropsFor(*this);
}

std::string monsterTexturePath(const Monster& m) {
    switch (m.type) {
        case Monster::Type::GreenSlime:
            return "Monster/GreenSlimeStatic.png";
        case Monster::Type::BlueSlime:
            return "Monster/GreenSlimeVariation1Static.png";
        case Monster::Type::RedSlime:
            return "Monster/GreenSlimeVariation2Static.png";
        case Monster::Type::Bug:
            return "Monster/Bug.png";
        case Monster::Type::Ghost:
            return "Monster/Ghost.png";
    }
    return "Monster/GreenSlimeStatic.png";
}

} // namespace Game
