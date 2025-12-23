#include "Game/Tool/Sword.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Controllers/Interact/TileSelector.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "cocos2d.h"
#include <cmath>

using namespace cocos2d;

namespace Game {

ToolKind Sword::kind() const { return ToolKind::Sword; }
std::string Sword::displayName() const { return std::string("Sword"); }

// 根据玩家解锁的矿井电梯层数选择不同的剑图标：
// - 没有解锁电梯时使用基础剑；
// - 每到 5 的倍数层数时，尝试显示对应的“进阶剑”贴图。
std::string Sword::iconPath() const {
    auto& ws = Game::globalState();
    if (ws.abyssElevatorFloors.empty()) {
        return std::string("Weapon/sword.png");
    }
    int maxFloor = 0;
    for (int f : ws.abyssElevatorFloors) {
        if (f > maxFloor) {
            maxFloor = f;
        }
    }
    if (maxFloor > 0 && maxFloor % 5 == 0) {
        return StringUtils::format("Weapon/sword_%d.png", maxFloor);
    }
    return std::string("Weapon/sword.png");
}

// 使用剑的逻辑较为简单：
// - 检查体力并扣除；
// - 刷新 HUD/热键栏；
// - 在玩家位置弹出 "Slash!" 提示；
// - 由于战斗系统可能在别处处理，这里只负责表现层。
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

// 计算剑的基础伤害：
// - 以常数 8 为基础；
// - 按解锁的电梯层数数量叠加额外伤害，每层 +2。
int Sword::baseDamage() {
    auto& ws = Game::globalState();
    std::size_t upgrades = ws.abyssElevatorFloors.size();
    int bonus = static_cast<int>(upgrades) * 2;
    return 8 + bonus;
}

// 根据玩家位置和朝向构建挥剑时覆盖的格子列表：
// - 内部复用 TileSelector::collectForwardFanTiles 的扇形选区算法；
// - includeSelf 控制是否包含玩家所在格子。
void Sword::buildHitTiles(Controllers::IMapController* map,
                          const Vec2& playerPos,
                          const Vec2& lastDir,
                          std::vector<std::pair<int,int>>& outTiles,
                          bool includeSelf) {
    outTiles.clear();
    if (!map) return;
    float ts = map->tileSize();
    Controllers::TileSelector::collectForwardFanTiles(
        playerPos,
        lastDir,
        [map](const Vec2& p, int& c, int& r) { map->worldToTileIndex(p, c, r); },
        [map](int c, int r) { return map->inBounds(c, r); },
        ts,
        includeSelf,
        outTiles);
}

}
