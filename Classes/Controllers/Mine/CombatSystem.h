/**
 * MineCombatController: 战斗控制器，处理鼠标左键攻击并与怪物/采矿交互。
 */
#pragma once

#include "cocos2d.h"
#include "Controllers/Mine/MonsterSystem.h"
#include "Controllers/Map/MineMapController.h"
#include <functional>

namespace Controllers {

class MineCombatController {
public:
    // 构造：注入矿洞地图/怪物系统和玩家位置、朝向查询回调。
    MineCombatController(MineMapController* map,
                          MineMonsterController* monsters,
                          std::function<cocos2d::Vec2()> getPlayerPos,
                          std::function<cocos2d::Vec2()> getLastDir)
    : _map(map), _monsters(monsters),
      _getPlayerPos(std::move(getPlayerPos)), _getLastDir(std::move(getLastDir)) {}

    // 处理鼠标左键按下：若当前选择剑则按扇形范围对怪物造成伤害。
    void onMouseDown(cocos2d::EventMouse* e);
    // 每帧更新占位：当前战斗逻辑全部在 onMouseDown 中完成，保留扩展点。
    void update(float dt) {}

private:
    MineMapController* _map = nullptr;
    MineMonsterController* _monsters = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
    std::function<cocos2d::Vec2()> _getLastDir;
};

} // namespace Controllers
