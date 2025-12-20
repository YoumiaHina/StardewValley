#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Systems/AnimalSystem.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"
#include "Game/Save/SaveSystem.h"

namespace Controllers {

static bool ensureWeatherChosenForToday() {
    auto& ws = Game::globalState();
    bool mismatch = (ws.weatherSeasonIndex != ws.seasonIndex) || (ws.weatherDayOfSeason != ws.dayOfSeason);
    if (!mismatch) return false;

    unsigned int seed = 0u;
    seed ^= static_cast<unsigned int>(ws.lastSaveSlot * 73856093);
    seed ^= static_cast<unsigned int>(ws.seasonIndex * 19349663);
    seed ^= static_cast<unsigned int>(ws.dayOfSeason * 83492791);
    int roll = static_cast<int>(seed % 100u);

    ws.isRaining = (roll < 30);
    ws.weatherSeasonIndex = ws.seasonIndex;
    ws.weatherDayOfSeason = ws.dayOfSeason;
    return true;
}

void GameStateController::update(float dt) {
    auto &ws = Game::globalState();
    if (ws.pendingPassOut) {
        if (_ui) _ui->refreshHUD();
        return;
    }
    ensureWeatherChosenForToday();
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
                ws.timeHour = 24;
                ws.pendingPassOut = true;
                timeChanged = true;
                ws.timeAccum = 0.0f;
                break;
            }
        }
        timeChanged = true;
    }
    if (timeChanged && _ui) _ui->refreshHUD();
    if (dayChanged) {
        ensureWeatherChosenForToday();
        if (_map) {
            if (_crop) { _crop->advanceCropsDaily(_map); }
            if (_animals) { _animals->advanceAnimalsDaily(); }
            _map->refreshMapVisuals();
            _map->refreshCropsVisuals();
        } else {
            if (_crop) { _crop->advanceCropsDaily(nullptr); }
            if (_animals) { _animals->advanceAnimalsDaily(); }
        }
        if (_ui) _ui->refreshHUD();
        std::string path = Game::currentSavePath();
        if (path.empty()) {
            if (ws.lastSaveSlot < 1) ws.lastSaveSlot = 1;
            if (ws.lastSaveSlot > 50) ws.lastSaveSlot = 50;
            path = Game::savePathForSlot(ws.lastSaveSlot);
        }
        Game::saveToFile(path);
    }
}

void GameStateController::sleepToNextMorning() {
    auto &ws = Game::globalState();
    ws.energy = ws.maxEnergy;
    ws.dayOfSeason += 1;
    if (ws.dayOfSeason > 30) {
        ws.dayOfSeason = 1;
        ws.seasonIndex = (ws.seasonIndex + 1) % 4;
    }
    ws.timeHour = 6;
    ws.timeMinute = 0;
    ws.timeAccum = 0.0f;
    ensureWeatherChosenForToday();
    if (_crop) {
        _crop->advanceCropsDaily(nullptr);
    }
    advanceAnimalsDailyWorldOnly();
    if (_ui) _ui->refreshHUD();
    std::string path = Game::currentSavePath();
    if (path.empty()) {
        if (ws.lastSaveSlot < 1) ws.lastSaveSlot = 1;
        if (ws.lastSaveSlot > 50) ws.lastSaveSlot = 50;
        path = Game::savePathForSlot(ws.lastSaveSlot);
    }
    Game::saveToFile(path);
}

}
