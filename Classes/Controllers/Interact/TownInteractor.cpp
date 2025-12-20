#include "Controllers/Interact/TownInteractor.h"
#include "Controllers/UI/UIController.h"

using namespace cocos2d;

namespace Controllers {

TownInteractor::SpaceAction TownInteractor::onSpacePressed() {
    if (_ui && _ui->isToolUpgradePanelVisible()) {
        _ui->toggleToolUpgradePanel(false);
        return SpaceAction::None;
    }
    if (!_map) return SpaceAction::None;
    Vec2 p = _getPlayerPos ? _getPlayerPos() : Vec2();
    if (_map->isNearDoor(p)) {
        return SpaceAction::EnterFarm;
    }
    if (_map->isNearToolUpdateArea(p)) {
        if (_ui) {
            bool visible = _ui->isToolUpgradePanelVisible();
            _ui->toggleToolUpgradePanel(!visible);
        }
        return SpaceAction::None;
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
