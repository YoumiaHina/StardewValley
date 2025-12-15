#include "Controllers/Interact/RoomInteractor.h"
#include "Controllers/Map/RoomMapController.h"
#include "Controllers/Systems/GameStateController.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Chest.h"

using namespace cocos2d;

namespace Controllers {

RoomInteractor::SpaceAction RoomInteractor::onSpacePressed() {
    if (!_map || !_inventory || !_ui || !_getPlayerPos) return SpaceAction::None;
    Vec2 p = _getPlayerPos();
    auto* room = static_cast<Controllers::RoomMapController*>(_map);

    // 出屋：改为 DoorToFarm 对象层判定
    if (_map->isNearFarmDoor(p)) {
        return SpaceAction::ExitHouse;
    }

    // 睡觉
    if (room->bedRect().containsPoint(p)) {
        if (_state) {
            _state->sleepToNextMorning();
        }
        _ui->popTextAt(p, "New Day", Color3B::WHITE);
        return SpaceAction::Slept;
    }

    if (_inventory->selectedKind() == Game::SlotKind::Item) {
        const auto &slot = _inventory->selectedSlot();
        if (slot.itemType == Game::ItemType::Chest) {
            Game::placeChestInRoom(room, _ui, _inventory, p);
        }
    }

    Game::openChestNearPlayer(_map, _ui, p, Vec2(0,-1));

    return SpaceAction::None;
}
// namespace Controllers
}
