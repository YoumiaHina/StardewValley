#include "Game/Tool/Pickaxe.h"
#include "Controllers/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Tile.h"
#include "Game/Drop.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Game {

ToolKind Pickaxe::kind() const { return ToolKind::Pickaxe; }
std::string Pickaxe::displayName() const { return std::string("Pickaxe"); }

std::string Pickaxe::use(Controllers::IMapController* map,
                         Controllers::CropSystem* /*crop*/,
                         std::function<Vec2()> getPlayerPos,
                         std::function<Vec2()> getLastDir,
                         Controllers::UIController* ui) {
    auto& ws = Game::globalState();
    int need = GameConfig::ENERGY_COST_PICKAXE;
    if (ws.energy < need) {
        if (getPlayerPos && ui) { ui->popTextAt(getPlayerPos(), std::string("Not enough energy"), Color3B::RED); }
        return std::string("");
    }
    Vec2 playerPos = getPlayerPos ? getPlayerPos() : Vec2();
    Vec2 lastDir = getLastDir ? getLastDir() : Vec2(0,-1);
    auto tgt = map->targetTile(playerPos, lastDir);
    int tc = tgt.first, tr = tgt.second;
    if (!map->inBounds(tc, tr)) return std::string("");
    auto current = map->getTile(tc, tr);
    std::string msg;
    if (current == Game::TileType::Rock) {
        map->setTile(tc, tr, Game::TileType::Soil);
        msg = std::string("Mine!");
        map->spawnDropAt(tc, tr, static_cast<int>(Game::ItemType::Stone), 1);
    } else {
        msg = std::string("Nothing");
    }
    ws.energy = std::max(0, ws.energy - need);
    if (ui) {
        ui->refreshHUD();
        ui->refreshHotbar();
        map->refreshMapVisuals();
        map->refreshDropsVisuals();
        ui->popTextAt(playerPos, msg, Color3B::YELLOW);
    }
    return msg;
}

}

