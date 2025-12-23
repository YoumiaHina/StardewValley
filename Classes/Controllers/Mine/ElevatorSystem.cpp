#include "Controllers/Mine/ElevatorSystem.h"
#include "Controllers/UI/UIController.h"

using namespace cocos2d;

namespace Controllers {

// buildPanel：
// 让 UIController 构建电梯面板并注册楼层选择回调。
// 这里不直接 new 任何 Cocos 节点，而是调用 UI 层暴露的方法：
// - buildElevatorPanel() 负责实际的 UI 布局与节点创建；
// - setElevatorFloorHandler(...) 注册一个回调函数，当玩家点击某个楼层按钮时，
//   UI 层会把楼层号传进来，本控制器再调用 _jumpToFloor 执行业务逻辑。
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
// - 切换可见性。玩家移动的锁定由 SceneBase/UIController 统一根据
//   isElevatorPanelVisible() 等接口综合判断，这里不再单独处理。
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
// - 隐藏电梯面板。玩家移动的解锁同样由 SceneBase/UIController 根据 UI 可视
//   状态统一更新，不在本类中直接修改 PlayerController。
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
