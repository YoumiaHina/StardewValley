#pragma once

#include "cocos2d.h"
#include <string>
#include "Game/EnvironmentObstacleBase.h"

namespace Game {

enum class TreeKind {
    Tree1 = 1,
    Tree2 = 2,
};

class Tree : public EnvironmentObstacleBase {
public:
    static Tree* create(const std::string& texture);
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

    void applyDamage(int amount) override;
    int hp() const override { return _hp; }
    bool dead() const override { return _hp <= 0; }

    cocos2d::Rect footRect() const override;
    void playDestructionAnimation(const std::function<void()>& onComplete) override;

private:
    cocos2d::Sprite* _sprite = nullptr;
    TreeKind _kind = TreeKind::Tree1;
    int _seasonIndex = 0;
    int _hp = 3;
    bool _falling = false;
};

struct TreePos {
    int c = 0;
    int r = 0;
    TreeKind kind = TreeKind::Tree1;
};

}
