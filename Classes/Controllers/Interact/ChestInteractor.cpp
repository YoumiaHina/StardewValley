#include "Controllers/Interact/ChestInteractor.h"
#include "Controllers/Map/RoomMapController.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/Map/BeachMapController.h"
#include "Controllers/Systems/ChestController.h"
#include "Game/WorldState.h"

using namespace cocos2d;

namespace Controllers {

void ChestInteractor::onLeftClick() {
    if (!_map || !_ui || !_inventory || !_getPlayerPos) return;
    Vec2 p = _getPlayerPos();
    Vec2 lastDir = _getLastDir ? _getLastDir() : Vec2(0,-1);
    if (_inventory->selectedKind() != Game::SlotKind::Item) return;
    const auto& slot = _inventory->selectedSlot();
    if (slot.itemType == Game::ItemType::Chest) {
        if (_map->isFarm()) {
            placeChestOnFarm(_map, _ui, _inventory, p, lastDir);
        } else {
            auto* room = dynamic_cast<Controllers::RoomMapController*>(_map);
            if (room) {
                placeChestInRoom(room, _ui, _inventory, p);
            } else {
                placeChestOnOutdoorMap(_map, _ui, _inventory, p);
            }
        }
    }
}

}
