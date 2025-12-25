#pragma once

#include "cocos2d.h"
#include <functional>
#include <string>
#include "Game/EnvironmentObstacle/EnvironmentObstacleBase.h"

namespace Game {

// 杂草障碍物节点：
// - 作用：在地图上展示并承载单个杂草实体的受击、HP 与销毁动画。
// - 职责边界：只管理单体杂草的可视化与局部状态；生成/存储/刷新由对应系统负责。
// - 主要协作对象：杂草系统驱动其受击与销毁，并在回调时触发掉落与移除。
class Weed : public EnvironmentObstacleBase {
public:
    // 工厂创建：使用指定贴图路径创建杂草实例。
    static Weed* create(const std::string& texture);
    // 初始化：加载贴图并构建内部 Sprite。
    bool initWithTexture(const std::string& texture);
    // 设置破碎态贴图（用于受击/销毁时切换贴图）。
    void setBrokenTexture(const std::string& texture);

    // 施加伤害：降低 HP，并在死亡时进入破坏流程。
    void applyDamage(int amount) override;
    // 获取当前生命值。
    int hp() const override { return _hp; }
    // 是否已死亡。
    bool dead() const override { return _hp <= 0; }

    // 获取脚底碰撞矩形（用于近地面阻挡与命中判定）。
    cocos2d::Rect footRect() const override;
    // 播放销毁动画并在完成后回调。
    void playDestructionAnimation(const std::function<void()>& onComplete) override;

private:
    cocos2d::Sprite* _sprite = nullptr;
    int _hp = 1;
    bool _breaking = false;
    std::string _brokenTexture;
};

// 杂草位置数据：用于系统层保存与重建杂草状态。
struct WeedPos {
    int c = 0;
    int r = 0;
};

}

