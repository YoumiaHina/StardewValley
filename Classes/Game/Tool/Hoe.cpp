#include "Game/Tool/Hoe.h"
#include "Controllers/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Crop.h"
#include "Game/Item.h"
#include "cocos2d.h"
#include <random>

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
    std::string msg;
    // 仅在农场地图进行耕地/收获逻辑；矿洞/室内直接返回 Nothing，避免访问不存在的瓦片数组
    if (!map->isFarm()) {
        msg = std::string("Nothing");
    } else {
        auto current = map->getTile(tc, tr);
        int idx = crop ? crop->findCropIndex(tc, tr) : -1;
        if (idx >= 0) {
            const auto& cp = crop->crops()[idx];
            bool canHarvest = crop->canHarvestAt(tc, tr);
            if (canHarvest) {
                bool yields = crop->yieldsOnHarvestAt(tc, tr);
                if (yields) {
                    auto produce = Game::produceItemFor(cp.type);
                    int minQty = 1;
                    int maxQty = 1;
                    if (produce == Game::ItemType::Parsnip) {
                        minQty = 3;
                        maxQty = 8;
                    } else {
                        minQty = 1;
                        maxQty = 5;
                    }
                    int qty = minQty;
                    if (maxQty > minQty) {
                        std::uniform_int_distribution<int> dist(minQty, maxQty);
                        static std::mt19937 eng{ std::random_device{}() };
                        qty = dist(eng);
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
                // 执行收获（可能拔除或转为成熟占位）
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

