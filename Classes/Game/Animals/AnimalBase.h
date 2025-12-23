#pragma once

#include "Game/Animals/Animal.h"

namespace Game {

// 动物行为静态信息基类：
// - 描述动物的移动速度、游走半径与贴图资源等常量。
// - 协作对象：AnimalSystem 通过该接口获取静态行为数据，并负责运行时状态与可视化管理。
class AnimalBase {
public:
    // 虚析构：允许通过基类引用销毁派生实例（当前派生为静态单例）。
    virtual ~AnimalBase() = default;
    // 返回动物类型枚举，用于被系统索引。
    virtual AnimalType type() const = 0;
    // 返回基础移动速度（单位：像素/秒或与地图缩放一致的速度）。
    virtual float moveSpeed() const = 0;
    // 返回游走半径（单位：tile，具体解释由系统统一换算）。
    virtual float wanderRadius() const = 0;
    // 返回动物贴图路径（用于创建 Sprite）。
    virtual const char* texturePath() const = 0;
};

}
