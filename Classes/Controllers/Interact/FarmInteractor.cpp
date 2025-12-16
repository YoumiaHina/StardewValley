#include "Controllers/Interact/FarmInteractor.h"
#include "Controllers/Map/FarmMapController.h"
#include "Game/Crop.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Chest.h"

using namespace cocos2d;

namespace Controllers {

FarmInteractor::SpaceAction FarmInteractor::onSpacePressed() {
    if (!_map || !_inventory || !_ui || !_getPlayerPos) return SpaceAction::None;
    Vec2 p = _getPlayerPos();
    // 进屋 / 进矿洞 / 去沙滩 / 去城镇
    if (_map->isNearDoor(p)) {
        return SpaceAction::EnterHouse;
    }
    if (_map->isNearMineDoor(p)) {
        return SpaceAction::EnterMine;
    }
    if (_map->isNearBeachDoor(p)) {
        return SpaceAction::EnterBeach;
    }
    if (_map->isNearTownDoor(p)) {
        return SpaceAction::EnterTown;
    }

    if (_inventory->selectedKind() == Game::SlotKind::Item) {
        const auto &slot = _inventory->selectedSlot();
        if (_animals && slot.itemQty > 0) {
            int consumed = 0;
            bool fed = _animals->tryFeedAnimal(p, slot.itemType, consumed);
            if (fed && consumed > 0) {
                bool ok = _inventory->consumeSelectedItem(consumed);
                if (ok) {
                    _ui->refreshHotbar();
                }
                return SpaceAction::None;
            }
        }
        // 播种逻辑保留（空格仍可播种）
        if (Game::isSeed(slot.itemType)) {
            auto lastDir = _getLastDir ? _getLastDir() : Vec2(0,-1);
            auto tgt = _map->targetTile(p, lastDir);
            int tc = tgt.first, tr = tgt.second;
            if (_map->inBounds(tc, tr) && !_map->isNearChest(_map->tileToWorld(tc,tr)) && (_crop ? _crop->findCropIndex(tc, tr) : -1) < 0) {
                auto t = _map->getTile(tc, tr);
                if (t == Game::TileType::Tilled || t == Game::TileType::Watered) {
                    int ct = static_cast<int>(Game::cropTypeFromSeed(slot.itemType));
                    auto type = static_cast<Game::CropType>(ct);
                    auto& ws = Game::globalState();
                    if (!Game::CropDefs::isSeasonAllowed(type, ws.seasonIndex)) {
                        _ui->popTextAt(_map->tileToWorld(tc,tr), "Out of season", Color3B::RED);
                        return SpaceAction::None;
                    }
                    if (_crop) { _crop->plantCrop(type, tc, tr); }
                    bool ok = _inventory->consumeSelectedItem(1);
                    if (ok) { _ui->refreshHotbar(); }
                    _map->refreshCropsVisuals();
                    _ui->popTextAt(_map->tileToWorld(tc,tr), "Planted", Color3B::YELLOW);
                }
            }
            return SpaceAction::None;
        }
        // 空格不再放置或打开箱子，避免与左键逻辑冲突
    }
    return SpaceAction::None;
}

void FarmInteractor::onLeftClick() {
    if (!_map || !_inventory || !_ui || !_getPlayerPos) return;
    if (!_chestInteractor) {
        _chestInteractor = new Controllers::ChestInteractor(_inventory, _map, _ui, _getPlayerPos, _getLastDir);
    }
    _chestInteractor->onLeftClick();
    Vec2 p = _getPlayerPos();
    auto lastDir = _getLastDir ? _getLastDir() : Vec2(0,-1);
    const auto &slot = _inventory->selectedSlot();
    if (Game::isSeed(slot.itemType)) {
        auto tgt = _map->targetTile(p, lastDir);
        int tc = tgt.first, tr = tgt.second;
        if (_map->inBounds(tc, tr) && !_map->isNearChest(_map->tileToWorld(tc,tr)) && (_crop ? _crop->findCropIndex(tc, tr) : -1) < 0) {
            auto t = _map->getTile(tc, tr);
            if (t == Game::TileType::Tilled || t == Game::TileType::Watered) {
                int ct = static_cast<int>(Game::cropTypeFromSeed(slot.itemType));
                auto type = static_cast<Game::CropType>(ct);
                auto& ws = Game::globalState();
                if (!Game::CropDefs::isSeasonAllowed(type, ws.seasonIndex)) {
                    _ui->popTextAt(_map->tileToWorld(tc,tr), "Out of season", Color3B::RED);
                    return;
                }
                if (_crop) { _crop->plantCrop(type, tc, tr); }
                bool ok = _inventory->consumeSelectedItem(1);
                if (ok) { _ui->refreshHotbar(); }
                _map->refreshCropsVisuals();
                _ui->popTextAt(_map->tileToWorld(tc,tr), "Planted", Color3B::YELLOW);
            }
        }
        return;
    }
}

} // namespace Controllers
