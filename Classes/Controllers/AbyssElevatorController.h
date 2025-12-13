/**
 * AbyssElevatorController: 深渊矿洞电梯面板（跳转已激活楼层）。
 */
#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <vector>
#include <functional>
#include "Controllers/Map/AbyssMapController.h"
#include "Controllers/AbyssMonsterController.h"
#include "Controllers/AbyssMiningController.h"

namespace Controllers {

class AbyssElevatorController {
public:
    AbyssElevatorController(AbyssMapController* map,
                            AbyssMonsterController* monsters,
                            AbyssMiningController* mining,
                            cocos2d::Scene* scene)
    : _map(map), _monsters(monsters), _mining(mining), _scene(scene) {}

    void buildPanel();
    void togglePanel();
    bool isPanelVisible() const { return _panel && _panel->isVisible(); }
    // 外部回调
    void setMovementLocker(std::function<void(bool)> cb) { _setMovementLocked = std::move(cb); }
    void setOnFloorChanged(std::function<void(int)> cb) { _onFloorChanged = std::move(cb); }

private:
    AbyssMapController* _map = nullptr;
    AbyssMonsterController* _monsters = nullptr;
    AbyssMiningController* _mining = nullptr;
    cocos2d::Scene* _scene = nullptr;
    cocos2d::ui::Layout* _panel = nullptr;
    std::function<void(bool)> _setMovementLocked;
    std::function<void(int)> _onFloorChanged;

    void _refreshButtons();
    void _jumpToFloor(int floor);
};

} // namespace Controllers
