#include "Controllers/Interact/RoomInteractor.h"
#include "Controllers/Map/RoomMapController.h"
#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Systems/FurnaceController.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"

using namespace cocos2d;

namespace Controllers {

RoomInteractor::SpaceAction RoomInteractor::onSpacePressed() {
    if (!_map || !_inventory || !_ui || !_getPlayerPos) return SpaceAction::None;
    Vec2 p = _getPlayerPos();
    auto* room = static_cast<Controllers::RoomMapController*>(_map);

    if (_map->isNearFarmDoor(p)) {
        return SpaceAction::ExitHouse;
    }

    if (room->bedRect().containsPoint(p)) {
        if (_state) {
            _state->sleepToNextMorning();
            auto& ws = Game::globalState();
            ws.lastScene = static_cast<int>(Game::SceneKind::Room);
        }
        _ui->popTextAt(_map->getPlayerPosition(p), "New Day", Color3B::WHITE);
        return SpaceAction::Slept;
    }

    return SpaceAction::None;
}

void RoomInteractor::onLeftClick() {
    if (!_map || !_inventory || !_ui || !_getPlayerPos) return;
    if (!_chestInteractor) {
        std::function<Vec2()> getLastDir = []() { return Vec2(0, -1); };
        _chestInteractor = new Controllers::ChestInteractor(_inventory, _map, _ui, _getPlayerPos, getLastDir);
    }
    _chestInteractor->onLeftClick();

    Vec2 p = _getPlayerPos();
    Vec2 lastDir(0, -1);
    auto* room = dynamic_cast<Controllers::RoomMapController*>(_map);
    if (room) {
        auto* furnace = room->furnaceController();
        if (furnace) {
            furnace->interactAt(p, lastDir);
        }
    }
}

// namespace Controllers
}
