#include "Controllers/Interact/MineInteractor.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Controllers {

MineInteractor::SpaceAction MineInteractor::onSpacePressed() {
    // 安全性：如果地图控制器还没就绪，直接返回“不做任何事”
    if (!_map) return SpaceAction::None;
    // 每次按键时实时获取玩家当前位置；若回调为空则退化为 (0,0)
    Vec2 p = _getPlayerPos ? _getPlayerPos() : Vec2();
    // 当前所在楼层（0 层一般是入口/电梯层；>0 为矿洞内部楼层）
    int floor = _map->currentFloor();
    // 在 0 层时，优先判定“电梯楼梯”（elestair），用于打开电梯面板
    if (floor <= 0 && _map->isNearElestair(p)) {
        return SpaceAction::UseElevator;
    }
    // 其它判定：玩家是否站在普通楼梯、矿洞门、农场门、回入口的门旁
    bool nearStairs = _map->isNearStairs(p);
    bool nearDoorArea = _map->isNearDoor(p);
    // 在矿洞内部楼层（>0）且靠近楼梯或门：向更深一层矿洞前进
    if (floor > 0 && (nearStairs || nearDoorArea)) {
        _map->descend(1);
        return SpaceAction::Descend;
    }
    // 在 0 层但不是电梯，只是普通楼梯：同样向下进入矿洞
    if (floor <= 0 && nearStairs) {
        _map->descend(1);
        return SpaceAction::Descend;
    }
    // 靠近农场门：返回农场
    if (_map->isNearFarmDoor(p)) {
        return SpaceAction::ReturnToFarm;
    }
    // 靠近“回入口”门：从当前楼层回到矿洞入口
    if (_map->isNearBack0(p)) {
        return SpaceAction::ReturnToEntrance;
    }
    // 不满足任何条件：本次空格不触发交互
    return SpaceAction::None;
}
// namespace Controllers
}
