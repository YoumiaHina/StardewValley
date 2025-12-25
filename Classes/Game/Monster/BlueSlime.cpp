#include "Game/Monster/SlimeMonsterBase.h"

namespace Game {

// BlueSlimeMonster：蓝色史莱姆怪物。
// - 相比 GreenSlime，血量和伤害更高（这里简单用 *3 做倍率）；
// - 仍然复用 SlimeMonsterBase 中的全部动画逻辑。
class BlueSlimeMonster : public SlimeMonsterBase {
public:
    // 初始化蓝色史莱姆的基础属性与掉落配置。
    BlueSlimeMonster() {
        def_.hp = 24 * 3;                // 血量比绿色史莱姆高 3 倍
        def_.dmg = 5 * 3;                // 攻击力比绿色史莱姆高 3 倍
        def_.def = 0;
        def_.searchRangeTiles = 4;
        def_.moveSpeed = 40.0f;
        def_.isCollisionAffected = true;
        def_.name = "Blue Slime";
        drops_.push_back(ItemType::Coal);
    }

    MonsterType monsterType() const override {
        return MonsterType::BlueSlime;
    }

    // 返回蓝色史莱姆的主体颜色。
    cocos2d::Color3B slimeColor() const override {
        return cocos2d::Color3B(120, 200, 255);
    }
};

// blueSlimeMonsterBehavior：同样返回一个静态的行为对象引用。
const MonsterBase& blueSlimeMonsterBehavior() {
    static BlueSlimeMonster inst;
    return inst;
}

} // namespace Game
