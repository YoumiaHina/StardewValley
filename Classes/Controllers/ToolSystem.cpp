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
        auto lastDir = Vec2(0,-1); // 由 PlayerController 更新光标；这里仅取目标格
        auto tgt = _map->targetTile(playerPos, lastDir);
        tc = tgt.first; tr = tgt.second;
    }
    if (!_map->inBounds(tc, tr)) return "";
    auto current = _map->getTile(tc, tr);

    switch (tool->type) {
        case Game::ToolType::Hoe: {
            int idx = _map->findCropIndex(tc, tr);
            if (idx >= 0) {
                _map->harvestCropAt(tc, tr);
                msg = "Harvest!";
            } else if (current == Game::TileType::Soil) {
                _map->setTile(tc,tr, Game::TileType::Tilled);
                msg = "Till!";
                _map->spawnDropAt(tc, tr, static_cast<int>(Game::ItemType::Fiber), 1);
            } else {
                msg = "Nothing";
            }
        } break;
        case Game::ToolType::WateringCan:
            if (current == Game::TileType::Tilled) { _map->setTile(tc,tr, Game::TileType::Watered); msg = "Water!"; }
            else msg = "Nothing";
            break;
        case Game::ToolType::Pickaxe:
            if (current == Game::TileType::Rock) { _map->setTile(tc,tr, Game::TileType::Soil); msg = "Mine!"; _map->spawnDropAt(tc, tr, static_cast<int>(Game::ItemType::Stone), 1); }
            else msg = "Nothing";
            break;
        case Game::ToolType::Axe:
            if (current == Game::TileType::Tree) { _map->setTile(tc,tr, Game::TileType::Soil); msg = "Chop!"; _map->spawnDropAt(tc, tr, static_cast<int>(Game::ItemType::Wood), 1); }
            else msg = "Nothing";
            break;
        default: msg = "Use"; break;
    }

    ws.energy = std::max(0, ws.energy - need);
    if (_ui && _getPlayerPos) {
        _ui->refreshHUD();
        _map->refreshMapVisuals();
        _map->refreshDropsVisuals();
        _ui->popTextAt(_getPlayerPos(), msg, Color3B::YELLOW);
    }
    return msg;
}

} // namespace Controllers