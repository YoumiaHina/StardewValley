#pragma once

#include "cocos2d.h"
#include "Game/Tool/ToolBase.h"

namespace Game {

// 玩家视图接口：
// - 作用：抽象玩家的可视化表现（朝向/移动动画/工具动作），供控制器驱动而不依赖具体实现。
// - 职责边界：只负责展示与动画状态切换，不承载输入判定、碰撞、背包等业务逻辑。
// - 主要协作对象：PlayerController/Scene 通过该接口驱动表现；具体实现由 PlayerView 提供。
class IPlayerView : public cocos2d::Node {
public:
    // 面朝方向（与贴图行列映射一致）。
    enum class Direction { DOWN = 0, RIGHT = 1, UP = 2, LEFT = 3 };

    // 虚析构：保证接口指针删除安全。
    virtual ~IPlayerView() = default;

    // 设置朝向（影响后续动画/工具动作的贴图选择）。
    virtual void setDirection(Direction dir) = 0;
    // 设置是否处于移动状态（影响行走/站立动画切换）。
    virtual void setMoving(bool moving) = 0;
    // 推进并刷新动画（由外部按帧调用）。
    virtual void updateAnimation(float dt) = 0;
    // 播放工具动作动画（根据工具种类与等级选择动作序列）。
    virtual void playToolAnimation(Game::ToolKind kind, int level) = 0;
};
}
