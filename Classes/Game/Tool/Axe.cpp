#include "Game/Tool/Axe.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"
#include "Controllers/Interact/TileSelector.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/SkillTree/SkillTreeSystem.h"
#include "Game/Tile.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Game {

ToolKind Axe::kind() const { return ToolKind::Axe; }
std::string Axe::displayName() const { return std::string("Axe"); }

std::string Axe::iconPath() const {
    int lv = level();
    std::string prefix;
    if (lv == 1) {
        prefix = "copper_";
    } else if (lv == 2) {
        prefix = "iron_";
    } else if (lv >= 3) {
        prefix = "gold_";
    }
    return std::string("Tool/") + prefix + "Axe.png";
}

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
    bool useFan = (level() >= 3 && ws.toolRangeModifier);
    std::vector<std::pair<int,int>> tiles;
    if (useFan && map) {
        Controllers::TileSelector::collectForwardFanTiles(
            playerPos,
            lastDir,
            [map](const Vec2& p, int& c, int& r) { map->worldToTileIndex(p, c, r); },
            [map](int c, int r) { return map->inBounds(c, r); },
            map->tileSize(),
            false,
            tiles);
    } else if (map) {
        auto tgt = map->targetTile(playerPos, lastDir);
        if (map->inBounds(tgt.first, tgt.second)) {
            tiles.push_back(tgt);
        }
    }
    if (tiles.empty()) return std::string("");

    bool acted = false;
    if (auto* sys = map->obstacleSystem(Controllers::ObstacleKind::Tree)) {
        int dmg = 1 + std::max(0, level());
        for (const auto& tile : tiles) {
            int tc = tile.first;
            int tr = tile.second;
            bool one = sys->damageAt(
                tc,
                tr,
                dmg,
                [map, ui](int c, int r, int itemType) {
                    if (!map) return;
                    int qty = 3;
                    if (itemType == static_cast<int>(Game::ItemType::Wood)) {
                        auto& skill = Game::SkillTreeSystem::getInstance();
                        qty = skill.adjustWoodDropQuantityForForestry(qty);
                        skill.addXp(Game::SkillTreeType::Forestry, skill.xpForForestryChop(qty));
                        if (ui && ui->isSkillTreePanelVisible()) {
                            ui->refreshSkillTreePanel();
                        }
                    }
                    map->spawnDropAt(c, r, itemType, qty);
                    map->refreshDropsVisuals();
                },
                [map](int c, int r, Game::TileType t) {
                    if (!map) return;
                    map->setTile(c, r, t);
                }
            );
            if (one) {
                acted = true;
            }
        }
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
