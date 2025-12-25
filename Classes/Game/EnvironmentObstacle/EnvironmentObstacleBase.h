#pragma once

#include "cocos2d.h"
#include <functional>

namespace Game {

// 环境障碍物基类：
// - 作用：抽象可被工具/攻击破坏的环境实体（HP、碰撞脚底区域、破坏动画）。
// - 职责边界：只定义“障碍物节点”的通用接口，不包含生成/存储/生命周期管理逻辑。
// - 主要协作对象：由对应的 *System 创建/挂接/销毁；工具/战斗模块通过系统驱动其受击与销毁表现。
class EnvironmentObstacleBase : public cocos2d::Node {
public:
    // 虚析构：保证基类指针删除安全。
    virtual ~EnvironmentObstacleBase() = default;

    // 施加伤害：更新内部 HP 并触发受击/破坏状态切换。
    virtual void applyDamage(int amount) = 0;
    // 获取当前生命值。
    virtual int hp() const = 0;
    // 是否已死亡（HP <= 0 或进入移除状态）。
    virtual bool dead() const = 0;

    // 获取脚底碰撞矩形（用于与玩家/工具判定的近地面区域）。
    virtual cocos2d::Rect footRect() const = 0;
    // 播放销毁动画；动画完成后回调，用于系统移除节点/触发掉落。
    virtual void playDestructionAnimation(const std::function<void()>& onComplete) = 0;
};

}
