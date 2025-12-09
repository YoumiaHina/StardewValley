#include "Controllers/ToolSystem.h"
#include "Game/GameConfig.h"
#include "Game/Crop.h"

using namespace cocos2d;

namespace Controllers {

std::string ToolSystem::useSelectedTool() {
    if (!_inventory || !_map) return "";
    const Game::Tool* tool = _inventory->selectedTool();
    if (!tool) return "";

    auto &ws = Game::globalState();
    auto costFor = [](Game::ToolType t){
        switch (t) {
            case Game::ToolType::Axe:         return GameConfig::ENERGY_COST_AXE;
            case Game::ToolType::Pickaxe:     return GameConfig::ENERGY_COST_PICKAXE;
            case Game::ToolType::Hoe:         return GameConfig::ENERGY_COST_HOE;
            case Game::ToolType::WateringCan: return GameConfig::ENERGY_COST_WATER;
            case Game::ToolType::FishingRod:  return 0;
            default: return 0;
        }
    };
    int need = costFor(tool->type);
    if (ws.energy < need) {
        if (_getPlayerPos && _ui) {
            _ui->popTextAt(_getPlayerPos(), "Not enough energy", Color3B::RED);
        }
        return "";
    }

    std::string msg;
    int tc = 0, tr = 0;
    if (_getPlayerPos) {
        auto playerPos = _getPlayerPos();
        auto lastDir = _getLastDir ? _getLastDir() : Vec2(0,-1);
        auto tgt = _map->targetTile(playerPos, lastDir);
        tc = tgt.first; tr = tgt.second;
    }
    if (!_map->inBounds(tc, tr)) return "";
    auto current = _map->getTile(tc, tr);

    switch (tool->type) {
        case Game::ToolType::Hoe: {
            int idx = _crop ? _crop->findCropIndex(tc, tr) : -1;
            if (idx >= 0) {
                const auto& cp = _crop->crops()[idx];
                if (cp.stage >= cp.maxStage) {
                    auto produce = Game::produceItemFor(cp.type);
                    int leftover = _inventory->addItems(produce, 1);
                    if (leftover > 0) {
                        _map->spawnDropAt(tc, tr, static_cast<int>(produce), leftover);
                        _map->refreshDropsVisuals();
                    }
                    if (_crop) { _crop->harvestCropAt(tc, tr); }
                    _map->refreshCropsVisuals();
                    if (_ui) { _ui->refreshHotbar(); }
                    msg = "Harvest!";
                } else {
                    msg = "Not ready";
                }
            } else if (current == Game::TileType::Soil) {
                _map->setTile(tc,tr, Game::TileType::Tilled);
                msg = "Till!";
            } else {
                msg = "Nothing";
            }
        } break;
        case Game::ToolType::WateringCan: {
            // 优先：在湖边并且水量未满时，直接补水
            bool nearLake = false;
            if (_getPlayerPos) {
                float s = _map->tileSize();
                float radius = s * GameConfig::LAKE_REFILL_RADIUS_TILES;
                nearLake = _map->isNearLake(_getPlayerPos(), radius);
            }
            if (nearLake && ws.water < ws.maxWater) {
                ws.water = ws.maxWater;
                msg = std::string("Refill! (") + std::to_string(ws.water) + "/" + std::to_string(ws.maxWater) + ")";
                // 补水不消耗能量
                need = 0;
                break;
            }

            // 正常浇水：需要有水且目标为 Tilled
            if (current == Game::TileType::Tilled) {
                if (ws.water <= 0) {
                    msg = nearLake ? "Refill first" : "No water";
                } else {
                    _map->setTile(tc,tr, Game::TileType::Watered);
                    ws.water = std::max(0, ws.water - GameConfig::WATERING_CAN_CONSUME);
                    msg = std::string("Water! (") + std::to_string(ws.water) + "/" + std::to_string(ws.maxWater) + ")";
                }
            } else {
                // 非耕地，若在湖边则提示可补水，否则无事发生
                msg = nearLake ? "Hold to Refill" : "Nothing";
            }
        } break;
        case Game::ToolType::FishingRod: {
            bool nearLake = false;
            if (_getPlayerPos) {
                float s = _map->tileSize();
                float radius = s * GameConfig::LAKE_REFILL_RADIUS_TILES;
                nearLake = _map->isNearLake(_getPlayerPos(), radius);
            }
            if (nearLake) {
                msg = "Fishing...";
                if (_onStartFishing && _getPlayerPos) {
                    _onStartFishing(_getPlayerPos());
                }
            } else {
                msg = "Need water";
            }
        } break;
        case Game::ToolType::Pickaxe:
            if (current == Game::TileType::Rock) { _map->setTile(tc,tr, Game::TileType::Soil); msg = "Mine!"; _map->spawnDropAt(tc, tr, static_cast<int>(Game::ItemType::Stone), 1); }
            else msg = "Nothing";
            break;
        case Game::ToolType::Axe: {
            bool acted = _map->damageTreeAt(tc, tr, 1);
            if (acted) { msg = "Chop!"; }
            else { msg = "Nothing"; }
        } break;
        default: msg = "Use"; break;
    }

    ws.energy = std::max(0, ws.energy - need);
    if (_ui && _getPlayerPos) {
        _ui->refreshHUD();
        _ui->refreshHotbar(); // 更新水壶水量蓝条
        _map->refreshMapVisuals();
        _map->refreshDropsVisuals();
        _ui->popTextAt(_getPlayerPos(), msg, Color3B::YELLOW);
    }
    return msg;
}

} // namespace Controllers
