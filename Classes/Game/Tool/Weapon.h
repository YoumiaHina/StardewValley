#pragma once

#include "Game/Tool/ToolBase.h"

namespace Game {

// Weapon 基类：用于承载“武器”这一类工具的共性。
// 当前只在代码层面把“武器”和普通工具区分开，后续可以在这里加上
// 通用武器属性（例如基础伤害、攻击速度、攻击范围等），由具体武器继承使用。
class Weapon : public ToolBase {
public:
    virtual ~Weapon() = default;
};

}
