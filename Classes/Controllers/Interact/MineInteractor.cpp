#include "Controllers/Interact/MineInteractor.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Controllers {

MineInteractor::SpaceAction MineInteractor::onSpacePressed() {
    if (!_map) return SpaceAction::None;
    Vec2 p = _getPlayerPos ? _getPlayerPos() : Vec2();
    int floor = _map->currentFloor();
    // 0 层电梯：优先判定 elestair
    if (floor <= 0 && _map->isNearElestair(p)) {
        return SpaceAction::UseElevator;
    }
    bool nearStairs = _map->isNearStairs(p);
    bool nearDoorArea = _map->isNearDoor(p);
    if (floor > 0 && (nearStairs || nearDoorArea)) {
        _map->descend(1);
        return SpaceAction::Descend;
    }
    if (floor <= 0 && nearStairs) {
        _map->descend(1);
        return SpaceAction::Descend;
    }
    if (_map->isNearFarmDoor(p)) {
        return SpaceAction::ReturnToFarm;
    }
    if (_map->isNearBack0(p)) {
        return SpaceAction::ReturnToEntrance;
    }
    return SpaceAction::None;
}
// namespace Controllers
}
