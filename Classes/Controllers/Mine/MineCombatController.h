/**
 * MineCombatController: 战斗控制器，处理鼠标左键攻击并与怪物/采矿交互。
 */
#pragma once

#include "cocos2d.h"
#include "Controllers/MineMonsterController.h"
#include "Controllers/Map/MineMapController.h"
#include <functional>

namespace Controllers {

class MineCombatController {
public:
    MineCombatController(MineMapController* map,
                          MineMonsterController* monsters,
                          std::function<cocos2d::Vec2()> getPlayerPos,
                          std::function<cocos2d::Vec2()> getLastDir)
    : _map(map), _monsters(monsters),
      _getPlayerPos(std::move(getPlayerPos)), _getLastDir(std::move(getLastDir)) {}

    void onMouseDown(cocos2d::EventMouse* e);
    void update(float dt) {}

private:
    MineMapController* _map = nullptr;
    MineMonsterController* _monsters = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
    std::function<cocos2d::Vec2()> _getLastDir;
};

} // namespace Controllers
