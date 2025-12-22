/**
 * MineMonsterController：矿洞刷怪系统。
 * - 作用：根据当前楼层与 TMX 中的怪物区域生成怪物，驱动其移动/攻击，
 *   并维护与地图系统的碰撞体同步，以及与世界节点上的可视精灵同步。
 * - 职责边界：只负责“怪物这一类环境实体”的运行时状态与行为编排，不参与
 *   具体渲染资源选择（贴图/动画帧由 Game::MonsterBase 派生类决定），也不
 *   直接操作 UI 或玩家节点，只通过 WorldState / MapController / DropSystem
 *   等接口协作。
 * - 主要协作对象：
 *   - Controllers::MineMapController：提供当前楼层、怪物出生点、碰撞检测、
 *     坐标转换和掉落生成入口。
 *   - Game::MonsterBase 系列：作为每类怪物的行为与视觉定义，暴露
 *     monsterInfoFor(type) 所返回的动画/属性接口。
 *   - Game::WorldState：记录玩家生命值与金币等全局战斗结果。
 *   - SkillTreeSystem / DropSystem 等：在怪物死亡时结算经验与掉落。
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
#include "Game/Monster/MonsterBase.h"
#include "cocos2d.h"

namespace Controllers {

class MineMonsterController {
public:
    // Monster：单只怪物的运行时状态（控制器内部使用，不对外暴露实现细节）。
    struct Monster {
        // 怪物类型（决定行为参数与动画资源）。
        Game::MonsterType type;
        // 当前世界坐标位置（用于移动与碰撞计算）。
        cocos2d::Vec2 pos;
        // 当前生命值（不持久化，只在本层战斗中使用）。
        int hp = 0;
        // 攻击冷却计时器（秒），为 0 时才允许再次造成伤害。
        float attackCooldown = 0.0f;
        // 当前移动速度向量，用于决定朝向与播放行走/静止动画。
        cocos2d::Vec2 velocity;
        // 绑定到 worldNode 的可视精灵指针，由控制器统一创建和销毁。
        cocos2d::Sprite* sprite = nullptr;
    };

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
