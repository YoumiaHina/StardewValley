/**
 * MineElevatorController：矿洞电梯控制器。
 * - 作用：在矿洞入口层（floor 0）构建并管理电梯 UI 面板，让玩家可以在已解锁
 *   的楼层之间快速跳转。
 * - 职责边界：只负责电梯面板自身的 UI 布局与楼层切换时的“地图/怪物状态重置 +
 *   回调通知”，不直接操作玩家节点或 HUD 文本；这些由外部注入的回调完成。
 * - 主要协作对象：
 *   - MineMapController：查询当前楼层与已激活电梯楼层列表，并负责切换楼层；
 *   - MineMonsterController：在楼层跳转时重置上一层怪物并为新层生成初始怪；
 *   - UIController / 场景：通过回调更新楼层标签、刷新快捷栏、定位玩家出生点。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include <functional>
#include "Controllers/Map/MineMapController.h"
#include "Controllers/Mine/MonsterSystem.h"

namespace Controllers {

class UIController;

class MineElevatorController {
public:
    // 构造函数：注入地图控制器、怪物控制器与 UI 控制器。
    // - map/monsters 为“业务模块”，负责实际楼层切换与怪物重置；
    // - ui 负责电梯面板的创建与显示隐藏，本类只通过接口与之交互。
    MineElevatorController(MineMapController* map,
                           MineMonsterController* monsters,
                           UIController* ui)
    : _map(map), _monsters(monsters), _ui(ui) {}

    // 构建电梯面板：在 UIController 中创建电梯 UI 组件并注册回调。
    void buildPanel();
    // 开关电梯面板：仅在入口层（floor 0）允许打开。
    void togglePanel();
    // 查询面板是否当前可见。
    bool isPanelVisible() const;
    // 设置楼层切换回调：楼层成功跳转后会调用 cb(floor)，由场景更新 HUD、
    // 玩家出生点等。std::function<void(int)> 可理解为“接受一个 int 参数、
    // 无返回值的函数指针”。
    void setOnFloorChanged(std::function<void(int)> cb) { _onFloorChanged = std::move(cb); }

private:
    MineMapController* _map = nullptr;              // 不拥有的指针：地图控制器
    MineMonsterController* _monsters = nullptr;     // 不拥有的指针：怪物控制器
    UIController* _ui = nullptr;                    // UI 控制器：真正持有电梯面板节点
    std::function<void(int)> _onFloorChanged;       // 楼层切换回调（供场景层使用）

    void _jumpToFloor(int floor);
};

} // namespace Controllers
