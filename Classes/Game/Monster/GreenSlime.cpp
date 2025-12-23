#include "Game/Monster/SlimeMonsterBase.h"

namespace Game {

// GreenSlimeMonster：绿色史莱姆怪物的具体配置。
// - 继承自 SlimeMonsterBase，只负责“数值”和“颜色”；
// - 动画细节（如何从贴图裁剪、如何循环播放）都在基类 SlimeMonsterBase 中实现。
class GreenSlimeMonster : public SlimeMonsterBase {
public:
    // 构造函数：用 C++ 的构造函数语法，在这里填充 def_ 和 drops_。
    // 注意：
    // - 构造函数名字与类名相同，没有返回值；
    // - 在 C 里通常用 init 函数，这里用的是 C++ 的面向对象写法。
    GreenSlimeMonster() {
        def_.hp = 24;                    // 基础血量
        def_.dmg = 5;                    // 攻击力
        def_.def = 0;                    // 防御
        def_.searchRangeTiles = 4;       // 搜索玩家的范围：4 格
        def_.moveSpeed = 40.0f;          // 移动速度（像素/秒）
        def_.isCollisionAffected = true; // 会被地图碰撞阻挡
        def_.name = "Green Slime";       // 显示名称
        // drops_ 是 std::vector<ItemType>，使用 push_back 追加一个掉落物类型。
        drops_.push_back(ItemType::Stone);
    }

    // 返回怪物类型：用于 MonsterSystem / 其它系统区分不同怪物。
    MonsterType monsterType() const override {
        return MonsterType::GreenSlime;
    }

    // 返回绿色史莱姆的主体颜色：
    // - cocos2d::Color3B 用 (r, g, b) 三个 0~255 整数组合表示颜色；
    // - 这里选的是偏亮的绿色。
    cocos2d::Color3B slimeColor() const override {
        return cocos2d::Color3B(120, 255, 120);
    }
};

// greenSlimeMonsterBehavior：返回一个“全局唯一”的 GreenSlimeMonster 行为对象。
// - static GreenSlimeMonster inst; 表示函数内的静态局部变量：
//   第一次调用时构造，之后所有调用都复用同一个实例；
// - 返回类型是 const MonsterBase&（常量引用），避免复制对象，又限制外部修改。
const MonsterBase& greenSlimeMonsterBehavior() {
    static GreenSlimeMonster inst;
    return inst;
}

} // namespace Game
