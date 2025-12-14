#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Systems/AnimalSystem.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"

namespace Controllers {

void GameStateController::update(float dt) {
    auto &ws = Game::globalState();
    bool timeChanged = false;
    bool dayChanged = false;
    ws.timeAccum += dt;
    while (ws.timeAccum >= GameConfig::REAL_SECONDS_PER_GAME_MINUTE) {
        ws.timeAccum -= GameConfig::REAL_SECONDS_PER_GAME_MINUTE;
        ws.timeMinute += 1;
        if (ws.timeMinute >= 60) {
            ws.timeMinute = 0;
            ws.timeHour += 1;
            if (ws.timeHour >= 24) {
                ws.timeHour = 0;
                ws.dayOfSeason += 1;
                if (ws.dayOfSeason > 30) { ws.dayOfSeason = 1; ws.seasonIndex = (ws.seasonIndex + 1) % 4; }
                dayChanged = true;
            }
        }
        timeChanged = true;
    }
    if (timeChanged && _ui) _ui->refreshHUD();
    if (dayChanged && _map) {
        if (_crop) { _crop->advanceCropsDaily(_map); }
        if (_animals) { _animals->advanceAnimalsDaily(); }
        _map->refreshMapVisuals();
        _map->refreshCropsVisuals();
    }
}

}
