#include "Game/Monster/SlimeMonsterBase.h"

namespace Game {

// 绿色史莱姆怪物：
// - 继承自 SlimeMonsterBase，共享史莱姆的动画逻辑
// - 通过 slimeColor 提供绿色调色与基础数值配置
class GreenSlimeMonster : public SlimeMonsterBase {
public:
    // 初始化绿色史莱姆的基础属性与掉落配置
    GreenSlimeMonster() {
        def_.hp = 24;
        def_.dmg = 5;
        def_.def = 0;
        def_.searchRangeTiles = 4;
        def_.moveSpeed = 40.0f;
        def_.isCollisionAffected = true;
        def_.name = "Green Slime";
        drops_.push_back(ItemType::Stone);
    }

    // 返回怪物类型枚举，用于系统层区分怪物种类
    Monster::Type monsterType() const override {
        return Monster::Type::GreenSlime;
    }

    // 返回绿色史莱姆的主体颜色（后续可在此调整色调）
    cocos2d::Color3B slimeColor() const override {
        return cocos2d::Color3B(120, 255, 120);
    }
};

const MonsterBase& greenSlimeMonsterBehavior() {
    static GreenSlimeMonster inst;
    return inst;
}

} // namespace Game
