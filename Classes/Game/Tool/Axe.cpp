#include "Game/Tool/Axe.h"
#include "Controllers/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Tile.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Game {

ToolKind Axe::kind() const { return ToolKind::Axe; }
std::string Axe::displayName() const { return std::string("Axe"); }

std::string Axe::use(Controllers::IMapController* map,
                     Controllers::CropSystem* /*crop*/,
                     std::function<Vec2()> getPlayerPos,
                     std::function<Vec2()> getLastDir,
                     Controllers::UIController* ui) {
    auto& ws = Game::globalState();
    int need = GameConfig::ENERGY_COST_AXE;
    if (ws.energy < need) {
        if (getPlayerPos && ui) {
            Vec2 p = getPlayerPos();
            if (map) p = map->getPlayerPosition(p);
            ui->popTextAt(p, std::string("Not enough energy"), Color3B::RED);
        }
        return std::string("");
    }
    Vec2 playerPos = getPlayerPos ? getPlayerPos() : Vec2();
    Vec2 lastDir = getLastDir ? getLastDir() : Vec2(0,-1);
    auto tgt = map->targetTile(playerPos, lastDir);
    int tc = tgt.first, tr = tgt.second;
    if (!map->inBounds(tc, tr)) return std::string("");

    bool acted = false;
    if (auto* sys = map->obstacleSystem(Controllers::ObstacleKind::Tree)) {
        acted = sys->damageAt(
            tc,
            tr,
            1,
            [map](int c, int r, int itemType) {
                if (!map) return;
                map->spawnDropAt(c, r, itemType, 3);
                map->refreshDropsVisuals();
            },
            [map](int c, int r, Game::TileType t) {
                if (!map) return;
                map->setTile(c, r, t);
            }
        );
    }
    std::string msg = acted ? std::string("Chop!") : std::string("Nothing");
    ws.energy = std::max(0, ws.energy - need);
    if (!ui) {
        map->refreshMapVisuals();
        map->refreshDropsVisuals();
    }
    if (ui) {
        ui->refreshHUD();
        ui->refreshHotbar();
        map->refreshMapVisuals();
        map->refreshDropsVisuals();
        ui->popTextAt(map ? map->getPlayerPosition(playerPos) : playerPos, msg, Color3B::YELLOW);
    }
    return msg;
}

}

