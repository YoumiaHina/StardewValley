/**
 * MineElevatorController: 矿洞电梯面板（跳转已激活楼层）。
 */
#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <vector>
#include <functional>
#include "Controllers/Map/MineMapController.h"
#include "Controllers/Mine/MonsterSystem.h"

namespace Controllers {

class MineElevatorController {
public:
    MineElevatorController(MineMapController* map,
                            MineMonsterController* monsters,
                            cocos2d::Scene* scene)
    : _map(map), _monsters(monsters), _scene(scene) {}

    void buildPanel();
    void togglePanel();
    bool isPanelVisible() const { return _panel && _panel->isVisible(); }
    // 外部回调
    void setMovementLocker(std::function<void(bool)> cb) { _setMovementLocked = std::move(cb); }
    void setOnFloorChanged(std::function<void(int)> cb) { _onFloorChanged = std::move(cb); }

private:
    MineMapController* _map = nullptr;
    MineMonsterController* _monsters = nullptr;
    cocos2d::Scene* _scene = nullptr;
    cocos2d::ui::Layout* _panel = nullptr;
    std::function<void(bool)> _setMovementLocked;
    std::function<void(int)> _onFloorChanged;

    void _refreshButtons();
    void _jumpToFloor(int floor);
};

} // namespace Controllers
