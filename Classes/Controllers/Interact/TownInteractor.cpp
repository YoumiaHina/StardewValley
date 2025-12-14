#include "Controllers/Interact/TownInteractor.h"

using namespace cocos2d;

namespace Controllers {

TownInteractor::SpaceAction TownInteractor::onSpacePressed() {
    if (!_map) return SpaceAction::None;
    Vec2 p = _getPlayerPos ? _getPlayerPos() : Vec2();
    if (_map->isNearDoor(p)) {
        return SpaceAction::EnterFarm;
    }
    if (_npc) {
        if (_npc->advanceDialogueIfActive()) {
            return SpaceAction::None;
        }
        _npc->handleTalkAt(p);
    }
    return SpaceAction::None;
}

} // namespace Controllers

