#include "Game/Tool/Sword.h"
#include "Controllers/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "cocos2d.h"

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
        if (getPlayerPos && ui) { ui->popTextAt(getPlayerPos(), std::string("Not enough energy"), Color3B::RED); }
        return std::string("");
    }
    Vec2 playerPos = getPlayerPos ? getPlayerPos() : Vec2();
    std::string msg = std::string("Slash!");
    ws.energy = std::max(0, ws.energy - need);
    if (ui) {
        ui->refreshHUD();
        ui->refreshHotbar();
        ui->popTextAt(playerPos, msg, Color3B::YELLOW);
    }
    // 地图可视无需变化，保持与镐子的调用风格一致
    if (map) { map->refreshMapVisuals(); }
    return msg;
}

}