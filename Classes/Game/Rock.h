#pragma once

#include "cocos2d.h"
#include <functional>
#include <string>
#include "Game/EnvironmentObstacleBase.h"

namespace Game {

enum class RockKind {
    Rock1 = 1,
    Rock2 = 2,
};

class Rock : public EnvironmentObstacleBase {
public:
    static Rock* create(const std::string& texture);
    bool initWithTexture(const std::string& texture);
    void setBrokenTexture(const std::string& texture);

    // 设置石头种类（rock1/rock2），并刷新贴图。
    void setKind(RockKind kind);
    // 获取石头种类（rock1/rock2）。
    RockKind kind() const;
    // 获取指定石头种类对应的贴图路径。
    static std::string texturePath(RockKind kind);

    void applyDamage(int amount) override;
    int hp() const override { return _hp; }
    bool dead() const override { return _hp <= 0; }

    cocos2d::Rect footRect() const override;
    void playDestructionAnimation(const std::function<void()>& onComplete) override;

private:
    cocos2d::Sprite* _sprite = nullptr;
    RockKind _kind = RockKind::Rock1;
    int _hp = 1;
    bool _breaking = false;
    std::string _brokenTexture;
};

struct RockPos {
    int c = 0;
    int r = 0;
    RockKind kind = RockKind::Rock1;
};

}
