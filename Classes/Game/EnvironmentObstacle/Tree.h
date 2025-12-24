#pragma once

#include "cocos2d.h"
#include <string>
#include "Game/EnvironmentObstacle/EnvironmentObstacleBase.h"

namespace Game {

// 树的种类：用于决定贴图与行为差异。
enum class TreeKind {
    Tree1 = 1,
    Tree2 = 2,
};

// 树障碍物节点：
// - 作用：在地图上展示并承载单棵树的受击、HP、季节贴图与倒下动画。
// - 职责边界：只管理单体树的可视化与局部状态；生成/存储/刷新由对应系统负责。
// - 主要协作对象：树木系统驱动其受击与销毁；季节/天气系统提供季节索引来源。
class Tree : public EnvironmentObstacleBase {
public:
    // 工厂创建：使用指定贴图路径创建树实例。
    static Tree* create(const std::string& texture);
    // 初始化：加载贴图并构建内部 Sprite。
    bool initWithTexture(const std::string& texture);

    // 设置树的种类（tree1/tree2），并刷新贴图。
    void setKind(TreeKind kind);
    // 获取树的种类（tree1/tree2）。
    TreeKind kind() const;
    // 设置当前季节索引（0:春 1:夏 2:秋 3:冬），并刷新贴图。
    void setSeasonIndex(int seasonIndex);
    // 获取当前季节索引（0:春 1:夏 2:秋 3:冬）。
    int seasonIndex() const;
    // 获取指定树种类在指定季节下的贴图路径。
    static std::string texturePath(TreeKind kind, int seasonIndex);

    // 施加伤害：降低 HP，并在死亡时进入倒下/移除流程。
    void applyDamage(int amount) override;
    // 获取当前生命值。
    int hp() const override { return _hp; }
    // 是否已死亡。
    bool dead() const override { return _hp <= 0; }

    // 获取脚底碰撞矩形（用于近地面阻挡与命中判定）。
    cocos2d::Rect footRect() const override;
    // 播放倒下/销毁动画并在完成后回调。
    void playDestructionAnimation(const std::function<void()>& onComplete) override;

private:
    cocos2d::Sprite* _sprite = nullptr;
    TreeKind _kind = TreeKind::Tree1;
    int _seasonIndex = 0;
    int _hp = 3;
    bool _falling = false;
};

// 树的位置与类型数据：用于系统层保存与重建树木状态。
struct TreePos {
    int c = 0;
    int r = 0;
    TreeKind kind = TreeKind::Tree1;
};

}
