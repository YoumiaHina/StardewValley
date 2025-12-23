#include "Game/Monster/MonsterBase.h"

namespace Game {

// 这里仅做“分发”：根据 MonsterType 返回对应的怪物行为对象。
// 注意：
// - 函数返回的是 const MonsterBase& 引用，而不是值拷贝；
// - 这些行为对象在各自 cpp 文件里以 static 局部静态变量的方式存在（见 GreenSlime.cpp 等）。
const MonsterBase& greenSlimeMonsterBehavior();
const MonsterBase& blueSlimeMonsterBehavior();
const MonsterBase& redSlimeMonsterBehavior();
const MonsterBase& bugMonsterBehavior();
const MonsterBase& ghostMonsterBehavior();

// 根据枚举值选择具体的“行为配置”对象：
// - switch(MonsterType) 写法与 C 类似，只是枚举是强类型枚举；
// - 每个 case 返回一个具体怪物的 MonsterBase 实例引用。
const MonsterBase& monsterInfoFor(MonsterType type) {
    switch (type) {
        case MonsterType::GreenSlime: return greenSlimeMonsterBehavior();
        case MonsterType::BlueSlime:  return blueSlimeMonsterBehavior();
        case MonsterType::RedSlime:   return redSlimeMonsterBehavior();
        case MonsterType::Bug:        return bugMonsterBehavior();
        case MonsterType::Ghost:      return ghostMonsterBehavior();
    }
    // 理论上不会走到这里，给一个安全的默认值（绿色史莱姆）。
    return greenSlimeMonsterBehavior();
}

} // namespace Game
