#pragma once

#include "cocos2d.h"
#include "Game/Tool/ToolBase.h"

namespace Game {

class IPlayerView : public cocos2d::Node {
public:
    enum class Direction { DOWN = 0, RIGHT = 1, UP = 2, LEFT = 3 };

    virtual ~IPlayerView() = default;

    virtual void setDirection(Direction dir) = 0;
    virtual void setMoving(bool moving) = 0;
    virtual void updateAnimation(float dt) = 0;
    virtual void playToolAnimation(Game::ToolKind kind, int level) = 0;
};
}
