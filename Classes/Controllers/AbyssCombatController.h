/**
 * AbyssCombatController: 战斗控制器，处理鼠标左键攻击并与怪物/采矿交互。
 */
#pragma once

#include "cocos2d.h"
#include "Controllers/AbyssMonsterController.h"
#include "Controllers/AbyssMiningController.h"
#include <functional>

namespace Controllers {

class AbyssCombatController {
public:
    AbyssCombatController(AbyssMonsterController* monsters,
                          AbyssMiningController* mining,
                          std::function<cocos2d::Vec2()> getPlayerPos)
    : _monsters(monsters), _mining(mining), _getPlayerPos(std::move(getPlayerPos)) {}

    void onMouseDown(cocos2d::EventMouse* e);
    void update(float dt) {}

private:
    AbyssMonsterController* _monsters = nullptr;
    AbyssMiningController* _mining = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
};

} // namespace Controllers