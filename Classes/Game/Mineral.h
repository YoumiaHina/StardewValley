#pragma once

#include "cocos2d.h"
#include <string>
#include <functional>
#include "Game/Item.h"
#include "Game/EnvironmentObstacleBase.h"

namespace Game {

enum class MineralType {
    Rock,
    HardRock,
    HugeRock,
    CopperOre,
    IronOre,
    GoldOre
};

class Mineral : public EnvironmentObstacleBase {
public:
    static Mineral* create(const std::string& texture);
    bool initWithTexture(const std::string& texture);
    void setBrokenTexture(const std::string& texture);
    void setType(MineralType type) { _type = type; }
    MineralType type() const { return _type; }

    void applyDamage(int amount) override;
    int hp() const override { return _hp; }
    bool dead() const override { return _hp <= 0; }

    cocos2d::Rect footRect() const override;
    cocos2d::Size spriteContentSize() const;
    void playDestructionAnimation(const std::function<void()>& onComplete) override;

private:
    cocos2d::Sprite* _sprite = nullptr;
    int _hp = 1;
    bool _breaking = false;
    std::string _brokenTexture;
    MineralType _type = MineralType::Rock;
};

struct MineralData {
    MineralType type;
    cocos2d::Vec2 pos;
    int hp = 1;
    int sizeTiles = 1;
    std::string texture;
};

ItemType mineralDropItem(MineralType type);

}

