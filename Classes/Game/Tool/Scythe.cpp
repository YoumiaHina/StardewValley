#include "Game/Tool/Scythe.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/Environment/EnvironmentObstacleSystemBase.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/Tile.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Game {

ToolKind Scythe::kind() const { return ToolKind::Scythe; }
std::string Scythe::displayName() const { return std::string("Scythe"); }

// 镰刀图标路径固定，不随等级变化。
std::string Scythe::iconPath() const {
    return std::string("Tool/Scythe.png");
}

// 使用镰刀的逻辑：
// 1. 检查体力；不足时弹出提示并返回；
// 2. 只对玩家面前的一个格子进行判定；
// 3. 若该格子存在 Weed 障碍，则通过 damageAt 破坏并生成一个掉落；
// 4. 扣除体力，刷新 HUD 与地图显示。
std::string Scythe::use(Controllers::IMapController* map,
                        Controllers::CropSystem* /*crop*/,
                        std::function<Vec2()> getPlayerPos,
                        std::function<Vec2()> getLastDir,
                        Controllers::UIController* ui) {
    auto& ws = Game::globalState();
    int need = GameConfig::ENERGY_COST_SCYTHE;
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

    bool acted = false;
    if (auto* sys = map->obstacleSystem(Controllers::ObstacleKind::Weed)) {
        acted = sys->damageAt(
            tc,
            tr,
            1,
            [map](int c, int r, int itemType) {
                if (!map) return;
                map->spawnDropAt(c, r, itemType, 1);
                map->refreshDropsVisuals();
            },
            nullptr
        );
    }
    std::string msg = acted ? std::string("Cut!") : std::string("Nothing");
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
