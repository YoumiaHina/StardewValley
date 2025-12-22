#include "Controllers/Mine/ElevatorSystem.h"
#include "Controllers/UI/UIController.h"

using namespace cocos2d;

namespace Controllers {

// buildPanel：
// 构建电梯面板的根节点与基础样式。仅在 _panel 为空时执行一次：
// - 创建半透明深色背景的 Layout；
// - 将其放置在屏幕中央，初始设置为隐藏；
// - 挂到场景节点树上（高 zOrder，确保浮在游戏画面之上）；
// - 调用 _refreshButtons() 根据当前已解锁的楼层生成按钮。
void MineElevatorController::buildPanel() {
    if (!_ui) return;
    _ui->buildElevatorPanel();
    _ui->setElevatorFloorHandler([this](int floor) {
        _jumpToFloor(floor);
    });
}

// togglePanel：
// 打开或关闭电梯面板（仅允许在入口层 floor <= 0 时打开）。
// 调用流程：
// - 若还未构建面板，则先调用 buildPanel；
// - 若当前楼层 > 0，则强制隐藏面板并直接返回（禁止在非入口层使用电梯）；
// - 刷新按钮列表，确保楼层变化后 UI 反映最新的解锁状态；
// - 切换可见性，并通过 _setMovementLocked 回调通知外部是否需要锁定玩家移动。
void MineElevatorController::togglePanel() {
    if (!_ui || !_map) return;
    if (_map->currentFloor() > 0) {
        _ui->toggleElevatorPanel(false);
        return;
    }
    buildPanel();
    auto floors = _map->getActivatedElevatorFloors();
    _ui->refreshElevatorPanel(floors);
    bool newVisible = !_ui->isElevatorPanelVisible();
    _ui->toggleElevatorPanel(newVisible);
}

bool MineElevatorController::isPanelVisible() const {
    return _ui && _ui->isElevatorPanelVisible();
}

// _jumpToFloor：
// 真正执行楼层跳转的地方：
// - 通过 MineMapController::setFloor 切换楼层；
// - 通知 MineMonsterController 清空上一层怪物，并为新楼层生成初始怪物波
//   （仅当楼层 > 0 时刷怪，入口层不刷怪）；
// - 调用外部注入的 _onFloorChanged(floor) 回调，让 UI/玩家位置等由场景层
//   统一管理；
// - 隐藏电梯面板并通过 _setMovementLocked(false) 解锁玩家移动。
void MineElevatorController::_jumpToFloor(int floor) {
    if (!_map) return;
    _map->setFloor(floor);
    if (_monsters) { _monsters->resetFloor(); }
    if (floor > 0) {
        if (_monsters) _monsters->generateInitialWave();
    }
    if (_onFloorChanged) _onFloorChanged(floor);
    if (_ui) {
        _ui->toggleElevatorPanel(false);
    }
}

} // namespace Controllers
