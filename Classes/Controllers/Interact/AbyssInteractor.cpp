#include "Controllers/Interact/AbyssInteractor.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Controllers {

AbyssInteractor::SpaceAction AbyssInteractor::onSpacePressed() {
    if (!_map) return SpaceAction::None;
    Vec2 p = _getPlayerPos ? _getPlayerPos() : Vec2();
    if (_map->isNearStairs(p)) {
        _map->descend(1);
        return SpaceAction::Descend;
    }
    if (_map->isNearFarmDoor(p)) {
        return SpaceAction::ReturnToFarm;
    }
    return SpaceAction::None;
}
// namespace Controllers
}
