#include "Controllers/Interact/BeachInteractor.h"

using namespace cocos2d;

namespace Controllers {

BeachInteractor::SpaceAction BeachInteractor::onSpacePressed() {
    if (!_map || !_inventory || !_ui || !_getPlayerPos) return SpaceAction::None;
    Vec2 p = _getPlayerPos();
    if (_map->isNearDoor(p)) {
        return SpaceAction::EnterFarm;
    }
    return SpaceAction::None;
}

} // namespace Controllers

