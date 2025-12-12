#include "Game/Tool/Hoe.h"
#include "Controllers/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Crop.h"
#include "Game/Item.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Game {

ToolKind Hoe::kind() const { return ToolKind::Hoe; }
std::string Hoe::displayName() const { return std::string("Hoe"); }

std::string Hoe::use(Controllers::IMapController* map,
                     Controllers::CropSystem* crop,
                     std::function<Vec2()> getPlayerPos,
                     std::function<Vec2()> getLastDir,
                     Controllers::UIController* ui) {
    auto& ws = Game::globalState();
    int need = GameConfig::ENERGY_COST_HOE;
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
    int idx = crop ? crop->findCropIndex(tc, tr) : -1;
    if (idx >= 0) {
        const auto& cp = crop->crops()[idx];
        bool canHarvest = crop->canHarvestAt(tc, tr);
        if (canHarvest) {
            bool yields = crop->yieldsOnHarvestAt(tc, tr);
            if (yields) {
                auto produce = Game::produceItemFor(cp.type);
                int leftover = ws.inventory ? ws.inventory->addItems(produce, 1) : 0;
                if (leftover > 0) {
                    map->spawnDropAt(tc, tr, static_cast<int>(produce), leftover);
                    map->refreshDropsVisuals();
                }
            }
            if (crop) { crop->harvestCropAt(tc, tr); }
            map->refreshCropsVisuals();
            if (ui) { ui->refreshHotbar(); }
            msg = yields ? std::string("Harvest!") : std::string("Uproot!");
        } else {
            msg = std::string("Not ready");
        }
    } else if (current == Game::TileType::Soil) {
        map->setTile(tc, tr, Game::TileType::Tilled);
        msg = std::string("Till!");
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

