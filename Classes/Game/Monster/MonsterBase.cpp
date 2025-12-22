#include "Game/Monster/MonsterBase.h"

namespace Game {

const MonsterBase& greenSlimeMonsterBehavior();
const MonsterBase& blueSlimeMonsterBehavior();
const MonsterBase& redSlimeMonsterBehavior();
const MonsterBase& bugMonsterBehavior();
const MonsterBase& ghostMonsterBehavior();

const MonsterBase& monsterInfoFor(MonsterType type) {
    switch (type) {
        case MonsterType::GreenSlime: return greenSlimeMonsterBehavior();
        case MonsterType::BlueSlime: return blueSlimeMonsterBehavior();
        case MonsterType::RedSlime: return redSlimeMonsterBehavior();
        case MonsterType::Bug: return bugMonsterBehavior();
        case MonsterType::Ghost: return ghostMonsterBehavior();
    }
    return greenSlimeMonsterBehavior();
}

} // namespace Game
