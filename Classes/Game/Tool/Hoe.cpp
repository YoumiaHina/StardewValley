#include "Game/Tool/Hoe.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/Environment/TreeSystem.h"
#include "Controllers/Environment/RockSystem.h"
#include "Controllers/Environment/WeedSystem.h"
#include "Controllers/Interact/TileSelector.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Crops/crop/CropBase.h"
#include "Game/Item.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Game {

ToolKind Hoe::kind() const { return ToolKind::Hoe; }
std::string Hoe::displayName() const { return std::string("Hoe"); }

// 根据等级返回不同前缀的图标路径（基础/铜/铁/金）。
std::string Hoe::iconPath() const {
    int lv = level();
    std::string prefix;
    if (lv == 1) {
        prefix = "copper_";
    } else if (lv == 2) {
        prefix = "iron_";
    } else if (lv >= 3) {
        prefix = "gold_";
    }
    return std::string("Tool/") + prefix + "Hoe.png";
}

// 使用锄头的逻辑：
// 1. 检查体力；若不足，则在玩家位置弹出 "Not enough energy"；
// 2. 根据等级和技能，决定使用单格还是前方扇形区域的格子；
// 3. 对每个格子：
//    - 若存在成熟作物，则尝试收获并将产物放入背包或生成掉落；
//    - 若是普通土壤 Soil 且没有树/石头/杂草阻挡，则把土壤翻耕成 Tilled/Watered；
// 4. 更新作物和地图可视，并刷新 HUD/热键栏。
std::string Hoe::use(Controllers::IMapController* map,
                     Controllers::CropSystem* crop,
                     std::function<Vec2()> getPlayerPos,
                     std::function<Vec2()> getLastDir,
                     Controllers::UIController* ui) {
    auto& ws = Game::globalState();
    int need = GameConfig::ENERGY_COST_HOE;
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
    std::string msg;
    if (!map->isFarm()) {
        msg = std::string("Nothing");
    } else {
        bool anyAction = false;
        for (const auto& tile : tiles) {
            int tc = tile.first;
            int tr = tile.second;
            auto current = map->getTile(tc, tr);
            int idx = crop ? crop->findCropIndex(tc, tr) : -1;
            if (idx >= 0) {
                bool canHarvest = crop->canHarvestAt(tc, tr);
                if (canHarvest) {
                    Game::ItemType produce = Game::ItemType::Parsnip;
                    int qty = 0;
                    bool yields = false;
                    bool did = crop ? crop->harvestByHoeAt(tc, tr, level(), produce, qty, yields) : false;
                    if (!did) {
                        if (msg.empty()) msg = std::string("Nothing");
                        continue;
                    }
                    if (yields) {
                        if (ui && ui->isSkillTreePanelVisible()) {
                            ui->refreshSkillTreePanel();
                        }
                        int leftover = 0;
                        if (ws.inventory) {
                            leftover = ws.inventory->addItems(produce, qty);
                        }
                        if (leftover > 0) {
                            map->spawnDropAt(tc, tr, static_cast<int>(produce), leftover);
                            map->refreshDropsVisuals();
                        }
                    }
                    map->refreshCropsVisuals();
                    if (ui) { ui->refreshHotbar(); }
                    msg = yields ? std::string("Harvest!") : std::string("Uproot!");
                    anyAction = true;
                } else {
                    if (msg.empty()) {
                        msg = std::string("Not ready");
                    }
                }
            } else if (current == Game::TileType::Soil) {
                bool blocked = false;
                if (auto* sys = map->obstacleSystem(Controllers::ObstacleKind::Tree)) {
                    auto* ts = dynamic_cast<Controllers::TreeSystem*>(sys);
                    if (ts && ts->findTreeAt(tc, tr)) blocked = true;
                }
                if (!blocked) {
                    if (auto* sys = map->obstacleSystem(Controllers::ObstacleKind::Rock)) {
                        auto* rs = dynamic_cast<Controllers::RockSystem*>(sys);
                        if (rs && rs->findRockAt(tc, tr)) blocked = true;
                    }
                }
                if (!blocked) {
                    if (auto* sys = map->obstacleSystem(Controllers::ObstacleKind::Weed)) {
                        auto* rs = dynamic_cast<Controllers::WeedSystem*>(sys);
                        if (rs && rs->findWeedAt(tc, tr)) blocked = true;
                    }
                }
                if (blocked) {
                    if (msg.empty()) {
                        msg = std::string("Nothing");
                    }
                } else {
                    map->setTile(tc, tr, ws.isRaining ? Game::TileType::Watered : Game::TileType::Tilled);
                    msg = std::string("Till!");
                    anyAction = true;
                }
            }
        }
        if (!anyAction && msg.empty()) {
            msg = std::string("Nothing");
        }
    }
    ws.energy = std::max(0, ws.energy - need);
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
