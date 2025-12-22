#include "Game/Monster/MonsterBase.h"

namespace Game {

const MonsterBase& greenSlimeMonsterBehavior();
const MonsterBase& blueSlimeMonsterBehavior();
const MonsterBase& redSlimeMonsterBehavior();
const MonsterBase& bugMonsterBehavior();
const MonsterBase& ghostMonsterBehavior();

void MonsterBase::initInstance(Monster& m) const {
    m.isCollisionAffected = def_.isCollisionAffected;
    m.hp = def_.hp;
    m.maxHp = def_.hp;
    m.dmg = def_.dmg;
    m.def = def_.def;
    m.searchRangeTiles = def_.searchRangeTiles;
    m.moveSpeed = def_.moveSpeed;
    m.name = def_.name ? def_.name : "";
}

std::vector<ItemType> MonsterBase::dropsFor(const Monster& m) const {
    std::vector<ItemType> result = drops_;
    if (m.elite && !result.empty()) {
        result.insert(result.end(), result.begin(), result.end());
    }
    return result;
}

const MonsterBase& monsterInfoFor(Monster::Type type) {
    switch (type) {
        case Monster::Type::GreenSlime: return greenSlimeMonsterBehavior();
        case Monster::Type::BlueSlime: return blueSlimeMonsterBehavior();
        case Monster::Type::RedSlime: return redSlimeMonsterBehavior();
        case Monster::Type::Bug: return bugMonsterBehavior();
        case Monster::Type::Ghost: return ghostMonsterBehavior();
    }
    return greenSlimeMonsterBehavior();
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
