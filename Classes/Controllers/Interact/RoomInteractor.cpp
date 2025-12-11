#include "Controllers/Interact/RoomInteractor.h"
#include "Controllers/Map/RoomMapController.h"
#include "Game/WorldState.h"

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
        auto &ws = Game::globalState();
        ws.energy = ws.maxEnergy;
        ws.dayOfSeason += 1;
        if (ws.dayOfSeason > 30) { ws.dayOfSeason = 1; ws.seasonIndex = (ws.seasonIndex + 1) % 4; }
        ws.timeHour = 6; ws.timeMinute = 0; ws.timeAccum = 0.0f;
        if (_crop) { _crop->advanceCropsDaily(_map); }
        _ui->refreshHUD();
        _ui->popTextAt(p, "New Day", Color3B::WHITE);
        return SpaceAction::Slept;
    }

    // 箱子交互：放置或存入
    if (_inventory->selectedKind() == Game::SlotKind::Item) {
        const auto &slot = _inventory->selectedSlot();
        if (slot.itemType == Game::ItemType::Chest) {
            Rect chestRect(p.x - 20, p.y - 20, 40, 40);
            bool blocked = room->doorRect().containsPoint(p) || chestRect.intersectsRect(room->bedRect());
            if (!blocked) {
                Game::Chest chest{ p, Game::Bag{} };
                _map->chests().push_back(chest);
                Game::globalState().houseChests = _map->chests();
                _inventory->removeItems(Game::ItemType::Chest, 1);
                _ui->refreshHotbar();
                _ui->popTextAt(p, "Placed Chest", Color3B::YELLOW);
            }
        } else {
            if (_map->isNearChest(p) && slot.itemQty > 0) {
                auto &chs = _map->chests();
                int idx = -1; float best = 1e9f;
                for (int i=0;i<(int)chs.size();++i) {
                    float d = p.distance(chs[i].pos);
                    if (d < best) { best = d; idx = i; }
                }
                if (idx >= 0) {
                    int qty = slot.itemQty;
                    chs[idx].bag.add(slot.itemType, qty);
                    Game::globalState().houseChests = chs;
                    _inventory->consumeSelectedItem(qty);
                    _ui->refreshHotbar();
                    _ui->popTextAt(chs[idx].pos, "Stored Items", Color3B::YELLOW);
                }
            }
        }
    }

    return SpaceAction::None;
}
// namespace Controllers
}
