// 本文件定义了“怪物”的通用接口（MonsterBase）以及怪物枚举类型。
// - enum class：强类型枚举，用来区分怪物种类；
// - struct / class：和 C 里的 struct 类似，只是默认权限不同；
// - 虚函数 virtual：子类可以重写，用来做“多态”——不同怪物有不同动画；
// - std::vector：C++ 标准库里的“动态数组”，自动扩容、自动管理内存。
#pragma once

// cocos2d.h：引入 cocos2d-x 引擎的所有常用类型，例如 Sprite、Vec2 等。
#include "cocos2d.h"
// <string> / <vector> / <functional>：C++ 标准库头文件，分别提供字符串、
// 动态数组和函数回调（std::function）等工具。
#include <string>
#include <vector>
#include <functional>
// Game/Item.h：定义了 ItemType（物品类型枚举），怪物死亡掉落会用到。
#include "Game/Item.h"

namespace Game {

// enum class：强类型枚举，和 C 里的 enum 类似，但不会隐式转换为 int。
// 使用方法：
// - 声明：MonsterType t = MonsterType::GreenSlime;
// - 对比：if (t == MonsterType::Bug) { ... }
enum class MonsterType { GreenSlime, BlueSlime, RedSlime, Bug, Ghost };

// struct：和 C 里结构体类似，这里用来保存怪物的数值配置。
// 默认成员都是 public，可在代码里直接通过 def_.hp 等访问。
struct MonsterDef {
    int hp = 0;                 // HP：生命值（血量）
    int dmg = 0;                // dmg：攻击力（一次攻击能造成多少伤害）
    int def = 0;                // def：防御力（抵挡一部分伤害）
    int searchRangeTiles = 0;   // searchRangeTiles：怪物“感知”玩家的范围（单位：格子数）
    float moveSpeed = 0.0f;     // moveSpeed：移动速度（像素/秒）
    bool isCollisionAffected = true; // 是否会被碰撞影响（true 表示会被地图/其它怪挡住）
    const char* name = nullptr; // 怪物名称，使用 const char* 指向只读字符串常量
};

// MonsterBase：怪物行为的抽象基类（接口）。
// - 只保存“每种怪物的配置”和“动画播放接口”，不包含具体的坐标或血量状态；
// - 真正的怪物实体（在地图上的一只怪）由 MonsterSystem 记录位置/当前 HP；
// - 这里的接口更像“怪物模板/配置 + 播放动画的策略”。
class MonsterBase {
public:
    // 虚析构函数：virtual ~MonsterBase() = default;
    // - virtual 表示这是一个“虚函数”，通过基类指针删除子类对象时会调用子类析构；
    // - = default 表示使用编译器自动生成的默认实现，不写函数体。
    virtual ~MonsterBase() = default;

    // 纯虚函数（= 0）：要求每个子类必须实现，类似 C 里“约定要实现的回调”。
    // 这里返回怪物的种类，用于区分 GreenSlime / Bug / Ghost 等。
    virtual MonsterType monsterType() const = 0;

    // 怪物基础数值配置：
    // - 每个 MonsterBase 子类在构造函数里给 def_ 填入具体数值；
    // - def_ 是 public 成员，MonsterSystem 可以直接读取。
    MonsterDef def_;
    // 怪物死亡时可能掉落的物品类型列表：
    // - std::vector 是 C++ 的动态数组容器，支持 push_back 在末尾追加元素。
    std::vector<ItemType> drops_;

    // 播放“静止”状态的动画：站着不动的时候调用。
    // - cocos2d::Sprite*：Sprite 是 cocos2d-x 里“精灵”（带纹理的小图）；
    // - 函数参数是指针，传入的是“要播放动画的精灵对象”。
    virtual void playStaticAnimation(cocos2d::Sprite* sprite) const = 0;
    // 播放“移动”状态的动画：
    // - velocity：当前移动速度向量（Vec2，相当于包含 x、y 的结构体）；
    // - 不同方向可以选择不同的动画（例如向左/向右/向上）。
    virtual void playMoveAnimation(const cocos2d::Vec2& velocity, cocos2d::Sprite* sprite) const = 0;
    // 播放“死亡”动画：
    // - onComplete：死亡动画播放结束后要调用的回调函数（std::function<void()>）；
    //   就像 C 里的函数指针，只是更安全、可存储 lambda。
    virtual void playDeathAnimation(cocos2d::Sprite* sprite, const std::function<void()>& onComplete) const = 0;
};

// monsterInfoFor：根据怪物类型返回对应的 MonsterBase“行为配置对象”。
// 返回值是 const MonsterBase&（常量引用）：
// - 避免拷贝整个对象（更高效）；
// - 调用者不能修改内部数据，只能读取配置并调用播放动画的接口。
const MonsterBase& monsterInfoFor(MonsterType type);

} // namespace Game
