#include "Game/Tool/Pickaxe.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"
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
        if (getPlayerPos && ui) {
            Vec2 p = getPlayerPos();
            if (map) p = map->getPlayerPosition(p);
            ui->popTextAt(p, std::string("Not enough energy"), Color3B::RED);
        }
        return std::string("");
    }
    if (!map) return std::string("");
    Vec2 playerPos = getPlayerPos ? getPlayerPos() : Vec2();
    Vec2 lastDir = getLastDir ? getLastDir() : Vec2(0,-1);
    auto tgt = map->targetTile(playerPos, lastDir);
    int tc = tgt.first, tr = tgt.second;
    if (!map->inBounds(tc, tr)) return std::string("");

    bool hit = false;
    if (auto* sys = map->obstacleSystem(Controllers::ObstacleKind::Rock)) {
        hit = sys->damageAt(
            tc,
            tr,
            1,
            [map](int c, int r, int itemType) {
                if (!map) return;
                map->spawnDropAt(c, r, itemType, 1);
                map->refreshDropsVisuals();
            },
            [map](int c, int r, Game::TileType t) {
                if (!map) return;
                map->setTile(c, r, t);
            }
        );
    }

    std::string msg = std::string("Nothing");
    if (hit) {
        msg = std::string("Mine!");
    } else {
        auto current = map->getTile(tc, tr);
        if (current == Game::TileType::Rock) {
            map->setTile(tc, tr, Game::TileType::Soil);
            map->spawnDropAt(tc, tr, static_cast<int>(Game::ItemType::Stone), 1);
            msg = std::string("Mine!");
        }
    }
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
