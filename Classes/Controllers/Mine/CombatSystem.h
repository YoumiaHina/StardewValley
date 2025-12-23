/**
 * MineCombatController：矿洞战斗控制器。
 * - 作用：监听鼠标左键点击事件，根据当前选中的工具（主要是剑）来构建攻击范围，
 *   并将伤害请求转发给 MineMonsterController / 环境系统。
 * - 职责边界：只负责“输入 → 攻击判定 → 调用顺序”的编排，不保存任何怪物状态，
 *   不直接修改世界状态（生命值/金币等由 MonsterSystem 与各自系统负责）。
 * - 主要协作对象：
 *   - MineMapController：提供瓦片/世界坐标转换，供武器构建攻击区域；
 *   - MineMonsterController：负责根据攻击区域对怪物结算伤害；
 *   - Game::Tool 系列（如 Sword）：定义每种武器的攻击形状与基础伤害。
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
    // - map        ：参与构建攻击瓦片范围（例如剑的扇形攻击）；
    // - monsters   ：真正执行伤害结算的怪物系统；
    // - getPlayerPos：查询玩家当前位置（世界坐标），作为攻击中心；
    // - getLastDir ：查询玩家最近朝向，用于决定攻击扇形方向；
    //   这里使用 std::function 包装回调，可理解为“带类型的函数指针”，
    //   外部可以传入 lambda、成员函数绑定等多种可调用对象。
    MineCombatController(MineMapController* map,
                          MineMonsterController* monsters,
                          std::function<cocos2d::Vec2()> getPlayerPos,
                          std::function<cocos2d::Vec2()> getLastDir)
    : _map(map), _monsters(monsters),
      _getPlayerPos(std::move(getPlayerPos)), _getLastDir(std::move(getLastDir)) {}

    // 处理鼠标左键按下：若当前选择剑则按扇形范围对怪物造成伤害。
    void onMouseDown(cocos2d::EventMouse* e);
    // 每帧更新：推进内部攻击冷却计时器等与时间相关的战斗状态。
    void update(float dt);

private:
    MineMapController* _map = nullptr;                 // 原始指针：由场景负责创建与销毁
    MineMonsterController* _monsters = nullptr;        // 指向怪物控制器，同样不负责释放
    std::function<cocos2d::Vec2()> _getPlayerPos;      // 查询玩家当前位置的回调
    std::function<cocos2d::Vec2()> _getLastDir;        // 查询玩家最近朝向的回调
    float _attackCooldownRemaining = 0.0f;             // 当前剩余攻击冷却时间（秒）
};

} // namespace Controllers
