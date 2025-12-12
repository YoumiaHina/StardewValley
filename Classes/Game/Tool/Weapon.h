#pragma once

#include "Game/Tool/ToolBase.h"

namespace Game {

// Weapon 基类：后续可扩展通用武器属性（伤害、速度、范围等）
class Weapon : public ToolBase {
public:
    virtual ~Weapon() = default;
};

}