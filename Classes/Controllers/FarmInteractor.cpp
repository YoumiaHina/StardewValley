#include "Controllers/FarmInteractor.h"
#include "Controllers/FarmMapController.h"
#include "Game/Crop.h"
#include "Game/WorldState.h"

using namespace cocos2d;

namespace Controllers {

FarmInteractor::SpaceAction FarmInteractor::onSpacePressed() {
    if (!_map || !_inventory || !_ui || !_getPlayerPos) return SpaceAction::None;
    Vec2 p = _getPlayerPos();
    // 进屋
    if (_map->isNearDoor(p)) {
        return SpaceAction::EnterHouse;
    }

    if (_inventory->selectedKind() == Game::SlotKind::Item) {
        const auto &slot = _inventory->selectedSlot();
        // 播种
        if (Game::isSeed(slot.itemType)) {
            auto lastDir = _getLastDir ? _getLastDir() : Vec2(0,-1);
            auto tgt = _map->targetTile(p, lastDir);
            int tc = tgt.first, tr = tgt.second;
            if (_map->inBounds(tc, tr) && !_map->isNearChest(_map->tileToWorld(tc,tr)) && (_crop ? _crop->findCropIndex(tc, tr) : -1) < 0) {
                auto t = _map->getTile(tc, tr);
                if (t == Game::TileType::Tilled || t == Game::TileType::Watered) {
                    int ct = static_cast<int>(Game::cropTypeFromSeed(slot.itemType));
                    if (_crop) { _crop->plantCrop(static_cast<Game::CropType>(ct), tc, tr); }
                    bool ok = _inventory->consumeSelectedItem(1);
                    if (ok) { _ui->refreshHotbar(); }
                    _map->refreshCropsVisuals();
                    _ui->popTextAt(_map->tileToWorld(tc,tr), "Planted", Color3B::YELLOW);
                }
            }
            return SpaceAction::None;
        }
        // 放置箱子
        if (slot.itemType == Game::ItemType::Chest) {
            auto lastDir = _getLastDir ? _getLastDir() : Vec2(0,-1);
            auto tgt = _map->targetTile(p, lastDir);
            int tc = tgt.first, tr = tgt.second;
            if (_map->inBounds(tc, tr)) {
                auto t = _map->getTile(tc, tr);
                if (t != Game::TileType::Rock && t != Game::TileType::Tree) {
                    Game::Chest chest{ _map->tileToWorld(tc, tr), Game::Bag{} };
                    _map->chests().push_back(chest);
                    Game::globalState().farmChests = _map->chests();
                    _map->refreshMapVisuals();
                    _ui->refreshHotbar();
                    _inventory->removeItems(Game::ItemType::Chest, 1);
                    _ui->popTextAt(_map->tileToWorld(tc,tr), "Placed Chest", Color3B::YELLOW);
                }
            }
            return SpaceAction::None;
        }
        // 存入最近箱子
        if (_map->isNearChest(p) && slot.itemQty > 0) {
            auto &chestsRef = _map->chests();
            int idx = -1; float best = 1e9f;
            for (int i=0;i<(int)chestsRef.size();++i) {
                float d = p.distance(chestsRef[i].pos);
                if (d < best) { best = d; idx = i; }
            }
            if (idx >= 0) {
                int qty = slot.itemQty;
                chestsRef[idx].bag.add(slot.itemType, qty);
                Game::globalState().farmChests = chestsRef;
                _inventory->consumeSelectedItem(qty);
                _ui->refreshHotbar();
                _ui->popTextAt(chestsRef[idx].pos, "Stored Items", Color3B::YELLOW);
            }
        }
    }
    return SpaceAction::None;
}

} // namespace Controllers
