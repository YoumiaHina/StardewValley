#include "Controllers/Interact/ChestInteractor.h"
#include "Controllers/Map/RoomMapController.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/Map/BeachMapController.h"
#include "Game/Chest.h"
#include "Game/WorldState.h"

using namespace cocos2d;

namespace Controllers {

void ChestInteractor::onLeftClick() {
    if (!_map || !_ui || !_inventory || !_getPlayerPos) return;
    Vec2 p = _getPlayerPos();
    Vec2 lastDir = _getLastDir ? _getLastDir() : Vec2(0,-1);
    bool opened = Game::openChestNearPlayer(_map, _ui, p, lastDir);
    if (opened) return;
    if (_inventory->selectedKind() != Game::SlotKind::Item) return;
    const auto& slot = _inventory->selectedSlot();
    if (slot.itemType == Game::ItemType::Chest) {
        if (_map->isFarm()) {
            Game::placeChestOnFarm(_map, _ui, _inventory, p, lastDir);
        } else {
            auto* room = dynamic_cast<Controllers::RoomMapController*>(_map);
            if (room) {
                Game::placeChestInRoom(room, _ui, _inventory, p);
            } else {
                auto& chs = _map->chests();
                if (chs.size() >= static_cast<std::size_t>(Game::Chest::MAX_PER_AREA)) {
                    _ui->popTextAt(p, "Too many chests", Color3B::RED);
                    return;
                }
                if (Game::isNearAnyChest(p, chs)) return;
                Game::Chest chest;
                chest.pos = p;
                chs.push_back(chest);
                auto& ws = Game::globalState();
                if (dynamic_cast<Controllers::TownMapController*>(_map)) {
                    ws.townChests = chs;
                } else if (dynamic_cast<Controllers::BeachMapController*>(_map)) {
                    ws.beachChests = chs;
                }
                _inventory->removeItems(Game::ItemType::Chest, 1);
                _ui->refreshHotbar();
                _ui->popTextAt(p, "Placed Chest", Color3B::YELLOW);
                _map->refreshMapVisuals();
            }
        }
    }
}

}
