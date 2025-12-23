#include "Game/Monster/SlimeMonsterBase.h"

namespace Game {

// RedSlimeMonster：红色史莱姆怪物。
// - 比蓝色史莱姆更强，这里简单用 *5 做血量/伤害倍率；
// - 在游戏平衡上可以进一步微调这些数值。
class RedSlimeMonster : public SlimeMonsterBase {
public:
    // 初始化红色史莱姆的基础属性与掉落配置。
    RedSlimeMonster() {
        def_.hp = 24 * 5;                // 血量：绿色史莱姆的 5 倍
        def_.dmg = 5 * 5;                // 攻击力：绿色史莱姆的 5 倍
        def_.def = 0;
        def_.searchRangeTiles = 4;
        def_.moveSpeed = 40.0f;
        def_.isCollisionAffected = true;
        def_.name = "Red Slime";
        drops_.push_back(ItemType::Stone);
    }

    MonsterType monsterType() const override {
        return MonsterType::RedSlime;
    }

    // 返回红色史莱姆的主体颜色。
    cocos2d::Color3B slimeColor() const override {
        return cocos2d::Color3B(255, 120, 120);
    }
};

// redSlimeMonsterBehavior：返回静态行为对象引用。
const MonsterBase& redSlimeMonsterBehavior() {
    static RedSlimeMonster inst;
    return inst;
}

} // namespace Game
