#include "Game/Tool/Sword.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "cocos2d.h"
#include <cmath>

using namespace cocos2d;

namespace Game {

ToolKind Sword::kind() const { return ToolKind::Sword; }
std::string Sword::displayName() const { return std::string("Sword"); }

std::string Sword::use(Controllers::IMapController* map,
                       Controllers::CropSystem* /*crop*/,
                       std::function<Vec2()> getPlayerPos,
                       std::function<Vec2()> /*getLastDir*/,
                       Controllers::UIController* ui) {
    auto& ws = Game::globalState();
    int need = GameConfig::ENERGY_COST_SWORD;
    if (ws.energy < need) {
        if (getPlayerPos && ui) {
            Vec2 p = getPlayerPos();
            if (map) p = map->getPlayerPosition(p);
            ui->popTextAt(p, std::string("Not enough energy"), Color3B::RED);
        }
        return std::string("");
    }
    Vec2 playerPos = getPlayerPos ? getPlayerPos() : Vec2();
    std::string msg = std::string("Slash!");
    ws.energy = std::max(0, ws.energy - need);
    if (ui) {
        ui->refreshHUD();
        ui->refreshHotbar();
        ui->popTextAt(map ? map->getPlayerPosition(playerPos) : playerPos, msg, Color3B::YELLOW);
    }
    // 地图可视无需变化，保持与镐子的调用风格一致
    if (map) { map->refreshMapVisuals(); }
    return msg;
}

int Sword::baseDamage() {
    return 8;
}

void Sword::buildHitTiles(Controllers::IMapController* map,
                          const Vec2& playerPos,
                          const Vec2& lastDir,
                          std::vector<std::pair<int,int>>& outTiles,
                          bool includeSelf) {
    outTiles.clear();
    if (!map) return;
    Vec2 dir = lastDir;
    if (dir.lengthSquared() < 0.0001f) {
        dir = Vec2(0, -1);
    }
    int pc = 0;
    int pr = 0;
    map->worldToTileIndex(playerPos, pc, pr);
    int dc = 0;
    int dr = 0;
    if (std::abs(dir.x) > std::abs(dir.y)) {
        dc = (dir.x > 0.1f) ? 1 : ((dir.x < -0.1f) ? -1 : 0);
    } else {
        dr = (dir.y > 0.1f) ? 1 : ((dir.y < -0.1f) ? -1 : 0);
    }
    if (dc == 0 && dr == 0) {
        dr = -1;
    }
    if (includeSelf && map->inBounds(pc, pr)) {
        outTiles.emplace_back(pc, pr);
    }
    if (dr != 0) {
        int fr = pr + dr;
        int tcCenter = pc;
        int trCenter = fr;
        for (int offset = -1; offset <= 1; ++offset) {
            int tc = tcCenter + offset;
            int tr = trCenter;
            if (map->inBounds(tc, tr)) {
                outTiles.emplace_back(tc, tr);
            }
        }
    } else if (dc != 0) {
        int fc = pc + dc;
        int tcCenter = fc;
        int trCenter = pr;
        for (int offset = -1; offset <= 1; ++offset) {
            int tc = tcCenter;
            int tr = trCenter + offset;
            if (map->inBounds(tc, tr)) {
                outTiles.emplace_back(tc, tr);
            }
        }
    }
}

}
