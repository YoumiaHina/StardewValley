#include "Controllers/Systems/FestivalController.h"
#include "Controllers/Map/IMapController.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"

namespace Controllers {

FestivalController::FestivalController(IMapController* map)
    : _map(map) {
    syncToMap(true);
    auto& ws = Game::globalState();
    _appliedSeasonIndex = ws.seasonIndex;
    _appliedDayOfSeason = ws.dayOfSeason;
}

void FestivalController::update(float) {
    auto& ws = Game::globalState();
    bool isNewDay = (ws.seasonIndex != _appliedSeasonIndex) || (ws.dayOfSeason != _appliedDayOfSeason);
    if (!isNewDay) return;

    _appliedSeasonIndex = ws.seasonIndex;
    _appliedDayOfSeason = ws.dayOfSeason;
    syncToMap(true);
}

bool FestivalController::isFestivalToday() const {
    const auto& ws = Game::globalState();
    return ws.dayOfSeason == GameConfig::FESTIVAL_DAY;
}

void FestivalController::syncToMap(bool force) {
    bool desired = isFestivalToday();
    if (!force && desired == _festivalActive) return;
    _festivalActive = desired;
    if (_map) {
        _map->setFestivalActive(_festivalActive);
    }
}

} // namespace Controllers

