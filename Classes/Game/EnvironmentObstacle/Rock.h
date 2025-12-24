#pragma once

#include "cocos2d.h"
#include <functional>
#include <string>
#include "Game/EnvironmentObstacle/EnvironmentObstacleBase.h"

namespace Game {

// 石头种类：用于决定贴图路径与表现差异。
enum class RockKind {
    Rock1 = 1,
    Rock2 = 2,
};

// 石头障碍物节点：
// - 作用：在地图上展示并承载单个石头实体的受击、HP 与销毁动画。
// - 职责边界：只管理单体石头的可视化与局部状态；生成/存储/刷新由对应系统负责。
// - 主要协作对象：石头系统驱动其受击与销毁，并在回调时触发掉落与移除。
class Rock : public EnvironmentObstacleBase {
public:
    // 工厂创建：使用指定贴图路径创建石头实例。
    static Rock* create(const std::string& texture);
    // 初始化：加载贴图并构建内部 Sprite。
    bool initWithTexture(const std::string& texture);
    // 设置破碎态贴图（用于受击/销毁时切换贴图）。
    void setBrokenTexture(const std::string& texture);

    // 设置石头种类（rock1/rock2），并刷新贴图。
    void setKind(RockKind kind);
    // 获取石头种类（rock1/rock2）。
    RockKind kind() const;
    // 获取指定石头种类对应的贴图路径。
    static std::string texturePath(RockKind kind);

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
    RockKind _kind = RockKind::Rock1;
    int _hp = 1;
    bool _breaking = false;
    std::string _brokenTexture;
};

// 石头的位置与类型数据：用于系统层保存与重建石头状态。
struct RockPos {
    int c = 0;
    int r = 0;
    RockKind kind = RockKind::Rock1;
};

}
