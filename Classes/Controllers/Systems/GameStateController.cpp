#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Systems/AnimalSystem.h"
#include "Controllers/Environment/TreeSystem.h"
#include "Controllers/Environment/RockSystem.h"
#include "Controllers/Environment/WeedSystem.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"
#include "Game/Save/SaveSystem.h"
#include <unordered_set>

namespace Controllers {

static bool ensureWeatherChosenForToday() {
    auto& ws = Game::globalState();
    bool mismatch = (ws.weatherSeasonIndex != ws.seasonIndex) || (ws.weatherDayOfSeason != ws.dayOfSeason);
    if (!mismatch) return false;

    if (ws.seasonIndex == 1 && ws.dayOfSeason == GameConfig::FESTIVAL_DAY) {
        ws.isRaining = false;
    } else {
        unsigned int seed = 0u;
        seed ^= static_cast<unsigned int>(ws.lastSaveSlot * 73856093);
        seed ^= static_cast<unsigned int>(ws.seasonIndex * 19349663);
        seed ^= static_cast<unsigned int>(ws.dayOfSeason * 83492791);
        int roll = static_cast<int>(seed % 100u);

        ws.isRaining = (roll < 30);
    }
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
        _crop->advanceCropsDaily(_map);
    }
    advanceAnimalsDaily(_map);
    if (!ws.farmTiles.empty() && ws.farmCols > 0 && ws.farmRows > 0) {
        const int cols = ws.farmCols;
        const int rows = ws.farmRows;

        auto keyOf = [](int c, int r) -> long long {
            return (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
        };

        std::unordered_set<long long> occupied;
        occupied.reserve(static_cast<size_t>(ws.farmTrees.size() + ws.farmRocks.size() + ws.farmWeeds.size()) + 16u);
        for (const auto& tp : ws.farmTrees) occupied.insert(keyOf(tp.c, tp.r));
        for (const auto& rp : ws.farmRocks) occupied.insert(keyOf(rp.c, rp.r));
        for (const auto& wp : ws.farmWeeds) occupied.insert(keyOf(wp.c, wp.r));

        auto getTile = [&ws, cols, rows](int c, int r) -> Game::TileType {
            if (c < 0 || r < 0 || c >= cols || r >= rows) return Game::TileType::NotSoil;
            size_t idx = static_cast<size_t>(r) * static_cast<size_t>(cols) + static_cast<size_t>(c);
            if (idx >= ws.farmTiles.size()) return Game::TileType::NotSoil;
            return ws.farmTiles[idx];
        };
        auto isOccupiedTile = [&occupied, &keyOf](int c, int r) -> bool {
            return occupied.count(keyOf(c, r)) != 0;
        };
        auto markOccupiedTile = [&occupied, &keyOf](int c, int r) {
            occupied.insert(keyOf(c, r));
        };

        TreeSystem::regrowNightlyWorldOnly(cols, rows, getTile, isOccupiedTile, markOccupiedTile);
        RockSystem::regrowNightlyWorldOnly(cols, rows, getTile, isOccupiedTile, markOccupiedTile);
        WeedSystem::regrowNightlyWorldOnly(cols, rows, getTile, isOccupiedTile, markOccupiedTile);
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
