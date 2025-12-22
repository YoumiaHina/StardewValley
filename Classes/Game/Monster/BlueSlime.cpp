#include "Game/Monster/SlimeMonsterBase.h"

namespace Game {

// 蓝色史莱姆怪物：
// - 继承自 SlimeMonsterBase，共享史莱姆的动画逻辑
// - 通过 slimeColor 提供蓝色调色与基础数值配置
class BlueSlimeMonster : public SlimeMonsterBase {
public:
    // 初始化蓝色史莱姆的基础属性与掉落配置
    BlueSlimeMonster() {
        def_.hp = 24 * 3;
        def_.dmg = 5 * 3;
        def_.def = 0;
        def_.searchRangeTiles = 4;
        def_.moveSpeed = 40.0f;
        def_.isCollisionAffected = true;
        def_.name = "Blue Slime";
        drops_.push_back(ItemType::Stone);
    }

    MonsterType monsterType() const override {
        return MonsterType::BlueSlime;
    }

    // 返回蓝色史莱姆的主体颜色（后续可在此调整色调）
    cocos2d::Color3B slimeColor() const override {
        return cocos2d::Color3B(120, 200, 255);
    }
};

const MonsterBase& blueSlimeMonsterBehavior() {
    static BlueSlimeMonster inst;
    return inst;
}

} // namespace Game
