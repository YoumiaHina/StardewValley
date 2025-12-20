#include "Game/Tool/FishingRod.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Game {

ToolKind FishingRod::kind() const { return ToolKind::FishingRod; }
std::string FishingRod::displayName() const { return std::string("Fishing Rod"); }

void FishingRod::setFishingStarter(std::function<void(const cocos2d::Vec2&)> cb) { _onStartFishing = std::move(cb); }

std::string FishingRod::iconPath() const {
    return std::string("Tool/FishingRod.png");
}

std::string FishingRod::use(Controllers::IMapController* map,
                            Controllers::CropSystem* /*crop*/,
                            std::function<Vec2()> getPlayerPos,
                            std::function<Vec2()> /*getLastDir*/,
                            Controllers::UIController* ui) {
    Vec2 playerPos = getPlayerPos ? getPlayerPos() : Vec2();
    float s = map ? map->tileSize() : static_cast<float>(GameConfig::TILE_SIZE);
    float radius = s * GameConfig::LAKE_REFILL_RADIUS_TILES;
    bool nearLake = map && map->isNearLake(playerPos, radius);
    std::string msg;
    if (nearLake) {
        msg = std::string("Fishing...");
        if (_onStartFishing) { _onStartFishing(playerPos); }
    } else {
        msg = std::string("Need water");
    }
    if (ui) {
        Vec2 headPos = map ? map->getPlayerPosition(playerPos + Vec2(0, s)) : (playerPos + Vec2(0, s));
        ui->popTextAt(headPos, msg, Color3B::YELLOW);
    }
    return msg;
}

}
