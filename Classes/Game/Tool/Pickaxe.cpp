#include "Game/Tool/Pickaxe.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/Map/FarmMapController.h"
#include "Controllers/Map/RoomMapController.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/Map/BeachMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/Systems/FurnaceController.h"
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

namespace {
Controllers::FurnaceController* furnaceControllerForMap(Controllers::IMapController* map) {
    if (!map) return nullptr;
    if (auto* farm = dynamic_cast<Controllers::FarmMapController*>(map)) return farm->furnaceController();
    if (auto* room = dynamic_cast<Controllers::RoomMapController*>(map)) return room->furnaceController();
    if (auto* town = dynamic_cast<Controllers::TownMapController*>(map)) return town->furnaceController();
    if (auto* beach = dynamic_cast<Controllers::BeachMapController*>(map)) return beach->furnaceController();
    return nullptr;
}
}

ToolKind Pickaxe::kind() const { return ToolKind::Pickaxe; }
std::string Pickaxe::displayName() const { return std::string("Pickaxe"); }

// 根据等级返回不同前缀的图标路径（基础/铜/铁/金）。
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

// 使用镐子的逻辑：
// 1. 检查体力；若不足，则在玩家位置弹出提示并立即返回；
// 2. 根据等级决定单格或扇形范围，通过 TileSelector 计算目标格子；
// 3. 若命中矿石系统，则对每个格子调用 damageAt，产生矿物掉落并增加挖矿经验；
// 4. 在农场上，还可以把没种作物的耕地/浇水耕地拍平成普通土壤；
// 5. 扣除体力，刷新 HUD 与地图可视，返回描述动作的文本。
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
    int dmg = 1 + std::max(0, level());
    if (auto* sys = map->obstacleSystem(Controllers::ObstacleKind::Rock)) {
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

    if (auto* furnace = furnaceControllerForMap(map)) {
        for (const auto& tile : tiles) {
            int tc = tile.first;
            int tr = tile.second;
            if (furnace->breakEmptyFurnaceAtTile(tc, tr, dmg)) {
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
