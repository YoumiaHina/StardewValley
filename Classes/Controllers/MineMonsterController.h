/**
 * MineMonsterController: 矿洞刷怪系统（主题关联、难度梯度、刷新规则）。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include <random>
#include <string>
#include <functional>
#include "Controllers/Map/MineMapController.h"
#include "Game/Item.h"
#include "Game/WorldState.h"
#include "Game/Monster.h"
#include "cocos2d.h"

namespace Controllers {

class MineMonsterController {
public:
    using Monster = Game::Monster;

    MineMonsterController(MineMapController* map,
                          cocos2d::Node* worldNode,
                          std::function<cocos2d::Vec2()> getPlayerPos)
    : _map(map), _worldNode(worldNode), _getPlayerPos(std::move(getPlayerPos)) {}

    ~MineMonsterController();

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
    std::function<cocos2d::Vec2()> _getPlayerPos;

public:
    void applyAreaDamage(const std::vector<std::pair<int,int>>& tiles, int baseDamage);
};

} // namespace Controllers
