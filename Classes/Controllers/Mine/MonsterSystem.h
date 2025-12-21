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

    // 生成当前楼层初始怪物波（按楼层和 MonsterArea 规则）。
    void generateInitialWave();
    // 每帧更新怪物移动/攻击、重生与碰撞体积。
    void update(float dt);
    // 重置当前楼层怪物和碰撞数据（切换楼层时调用）。
    void resetFloor();
    // 根据当前怪物列表刷新可视化精灵与调试绘制。
    void refreshVisuals();

    // 战斗交互：对 worldPos 附近最近怪物造成一次伤害。
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
