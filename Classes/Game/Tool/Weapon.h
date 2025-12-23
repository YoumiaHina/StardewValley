#pragma once

#include "Game/Tool/ToolBase.h"

namespace Game {

// Weapon 基类：用于承载“武器”这一类工具的共性。
// 职责：
// - 抽象出所有武器都会具备的基础属性：基础伤害、攻击冷却时间等；
// - 作为 ToolBase 的子类，被具体武器（如 Sword）继承实现；
// - 便于 MineCombatController 等系统通过统一接口读取武器数值。
class Weapon : public ToolBase {
public:
    // 返回武器的基础伤害数值（不含暴击/技能等临时加成）。
    // 由具体武器根据自身配置与全局状态（如楼层、技能树）实现。
    virtual int baseDamage() const = 0;
    // 返回两次攻击之间的最小冷却时间（秒）。
    // 控制最高攻击频率，由控制器在输入层按该冷却时间节流。
    virtual float attackCooldownSeconds() const = 0;
    virtual ~Weapon() = default;
};

}
