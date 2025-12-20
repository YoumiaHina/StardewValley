#include "Game/Tool/Pickaxe.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"
#include "Controllers/Interact/TileSelector.h"
#include "Game/WorldState.h"
#include "Game/SkillTree/SkillTreeSystem.h"
#include "Game/GameConfig.h"
#include "Game/Tile.h"
#include "Game/Drop.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Game {

ToolKind Pickaxe::kind() const { return ToolKind::Pickaxe; }
std::string Pickaxe::displayName() const { return std::string("Pickaxe"); }

std::string Pickaxe::iconPath() const {
    int lv = level();
    std::string prefix;
    if (lv == 1) {
        prefix = "copper_";
    } else if (lv == 2) {
        prefix = "iron_";
    } else if (lv >= 3) {
        prefix = "gold_";
    }
    return std::string("Tool/") + prefix + "Pickaxe.png";
}

std::string Pickaxe::use(Controllers::IMapController* map,
                         Controllers::CropSystem* crop,
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
    bool useFan = (level() >= 3 && ws.toolRangeModifier);
    std::vector<std::pair<int,int>> tiles;
    if (useFan) {
        Controllers::TileSelector::collectForwardFanTiles(
            playerPos,
            lastDir,
            [map](const Vec2& p, int& c, int& r) { map->worldToTileIndex(p, c, r); },
            [map](int c, int r) { return map->inBounds(c, r); },
            map->tileSize(),
            false,
            tiles);
    } else {
        auto tgt = map->targetTile(playerPos, lastDir);
        if (map->inBounds(tgt.first, tgt.second)) {
            tiles.push_back(tgt);
        }
    }
    if (tiles.empty()) return std::string("");

    bool hit = false;
    if (auto* sys = map->obstacleSystem(Controllers::ObstacleKind::Rock)) {
        int dmg = 1 + std::max(0, level());
        for (const auto& tile : tiles) {
            int tc = tile.first;
            int tr = tile.second;
            bool one = sys->damageAt(
                tc,
                tr,
                dmg,
                [map](int c, int r, int itemType) {
                    if (!map) return;
                    auto& skill = Game::SkillTreeSystem::getInstance();
                    Game::ItemType drop = static_cast<Game::ItemType>(itemType);
                    int qty = skill.adjustMiningDropQuantityForMining(drop, 1);
                    map->spawnDropAt(c, r, itemType, qty);
                    skill.addXp(Game::SkillTreeType::Mining, skill.xpForMiningBreak(drop, qty));
                    map->refreshDropsVisuals();
                },
                [map](int c, int r, Game::TileType t) {
                    if (!map) return;
                    map->setTile(c, r, t);
                }
            );
            if (one) {
                hit = true;
            }
        }
    }

    std::string msg = std::string("Nothing");
    bool flattened = false;
    if (hit) {
        msg = std::string("Mine!");
    } else if (map->isFarm()) {
        for (const auto& tile : tiles) {
            int tc = tile.first;
            int tr = tile.second;
            auto current = map->getTile(tc, tr);
            bool hasCrop = (crop && crop->findCropIndex(tc, tr) >= 0);
            if (!hasCrop && (current == Game::TileType::Tilled || current == Game::TileType::Watered)) {
                map->setTile(tc, tr, Game::TileType::Soil);
                flattened = true;
            }
        }
        if (flattened) {
            msg = std::string("Flatten!");
            map->refreshCropsVisuals();
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
