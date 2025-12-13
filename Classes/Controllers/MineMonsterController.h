/**
 * MineMonsterController: 矿洞刷怪系统（主题关联、难度梯度、刷新规则）。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include <random>
#include "Controllers/Map/MineMapController.h"
#include "Game/Item.h"
#include "Game/WorldState.h"
#include "cocos2d.h"

namespace Controllers {

class MineMonsterController {
public:
    struct Monster {
        enum class Type { RockSlime, BurrowBug, IceBat, IceMage, LavaCrab, LavaWarlock, BossGuardian };
        Type type;
        cocos2d::Vec2 pos;
        int hp; int maxHp; int dmg; int def;
        int searchRangeTiles;
        bool elite = false;
    };

    MineMonsterController(MineMapController* map, cocos2d::Node* worldNode)
    : _map(map), _worldNode(worldNode) {}

    void generateInitialWave();
    void update(float dt);
    void resetFloor();
    void refreshVisuals();

    // combat interactions
    void applyDamageAt(const cocos2d::Vec2& worldPos, int baseDamage);
    const std::vector<Monster>& monsters() const { return _monsters; }

private:
    MineMapController* _map = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    std::vector<Monster> _monsters;
    float _respawnAccum = 0.0f;
    cocos2d::DrawNode* _monsterDraw = nullptr;

    Monster makeMonsterForTheme(MineMapController::Theme theme);
};

} // namespace Controllers
