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
    MineElevatorController(MineMapController* map,
                           MineMonsterController* monsters,
                           UIController* ui)
    : _map(map), _monsters(monsters), _ui(ui) {}

    void buildPanel();
    void togglePanel();
    bool isPanelVisible() const;
    void setOnFloorChanged(std::function<void(int)> cb) { _onFloorChanged = std::move(cb); }

private:
    MineMapController* _map = nullptr;
    MineMonsterController* _monsters = nullptr;
    UIController* _ui = nullptr;
    std::function<void(int)> _onFloorChanged;

    void _jumpToFloor(int floor);
};

} // namespace Controllers
