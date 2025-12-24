#pragma once

#include "cocos2d.h"
#include <functional>
#include "Game/EnvironmentObstacle/EnvironmentObstacleBase.h"

namespace Game {

// 楼梯障碍物节点：
// - 作用：在地图上展示楼梯实体（通常用于触发换层/进入下一层）。
// - 职责边界：只管理楼梯节点的可视化与销毁表现；触发规则与换层逻辑由控制器/系统负责。
// - 主要协作对象：地图/楼层控制器检测玩家交互并触发换层；系统在换层时创建与清理楼梯节点。
class Stair : public EnvironmentObstacleBase {
public:
    // 工厂创建：使用指定贴图路径创建楼梯实例。
    static Stair* create(const std::string& texture);
    // 初始化：加载贴图并构建内部 Sprite。
    bool initWithTexture(const std::string& texture);

    // 施加伤害：楼梯通常不需要受击逻辑，此接口用于兼容基类约定。
    void applyDamage(int amount) override;
    // 获取当前生命值。
    int hp() const override { return _hp; }
    // 是否已死亡。
    bool dead() const override { return _hp <= 0; }

    // 获取脚底碰撞矩形（用于近地面阻挡与命中判定）。
    cocos2d::Rect footRect() const override;
    // 获取当前精灵的内容尺寸（用于外部布局/碰撞计算）。
    cocos2d::Size spriteContentSize() const;
    // 播放移除动画并在完成后回调。
    void playDestructionAnimation(const std::function<void()>& onComplete) override;

private:
    cocos2d::Sprite* _sprite = nullptr;
    int _hp = 1;
    bool _removing = false;
};

}
