#include "Controllers/Systems/WeatherController.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/Input/PlayerController.h"
#include "Game/WorldState.h"

namespace Controllers {

WeatherController::WeatherController(IMapController* map, cocos2d::Node* worldNode, PlayerController* playerController)
    : _map(map), _worldNode(worldNode), _playerController(playerController) {
    syncFromWorldState(true);
    auto& ws = Game::globalState();
    _appliedSeasonIndex = ws.seasonIndex;
    _appliedDayOfSeason = ws.dayOfSeason;
}

void WeatherController::update(float) {
    auto& ws = Game::globalState();
    bool isNewDay = (ws.seasonIndex != _appliedSeasonIndex) || (ws.dayOfSeason != _appliedDayOfSeason);

    WeatherKind desired = ws.isRaining ? WeatherKind::Rainy : WeatherKind::Sunny;
    if (isNewDay || desired != _weather) {
        _weather = desired;
        _appliedSeasonIndex = ws.seasonIndex;
        _appliedDayOfSeason = ws.dayOfSeason;
        syncFromWorldState(isNewDay || _weather == WeatherKind::Rainy);
    }
}

void WeatherController::setWeather(WeatherKind w) {
    auto& ws = Game::globalState();
    ws.isRaining = (w == WeatherKind::Rainy);
    ws.weatherSeasonIndex = ws.seasonIndex;
    ws.weatherDayOfSeason = ws.dayOfSeason;
    _weather = w;
    _appliedSeasonIndex = ws.seasonIndex;
    _appliedDayOfSeason = ws.dayOfSeason;
    syncFromWorldState(true);
}

void WeatherController::syncFromWorldState(bool forceApplyRainWatering) {
    auto& ws = Game::globalState();
    WeatherKind desired = ws.isRaining ? WeatherKind::Rainy : WeatherKind::Sunny;
    _weather = desired;

    if (_playerController) {
        _playerController->setMoveSpeedMultiplier(_weather == WeatherKind::Rainy ? 0.75f : 1.0f);
    }

    if (!_map || !_map->supportsWeather() || !_worldNode) return;

    if (_weather == WeatherKind::Rainy) {
        ensureDimLayer();
        if (_dimLayer) _dimLayer->setVisible(true);

        if (forceApplyRainWatering) {
            _map->setAllPlantableTilesWatered();
        }
    } else {
        if (_dimLayer) _dimLayer->setVisible(false);
    }
}

void WeatherController::ensureDimLayer() {
    if (_dimLayer || !_worldNode || !_map) return;
    cocos2d::Size s = _map->getContentSize();
    if (s.width <= 0.0f || s.height <= 0.0f) return;
    _dimLayer = cocos2d::LayerColor::create(cocos2d::Color4B(0, 0, 0, 90), s.width, s.height);
    if (!_dimLayer) return;
    _dimLayer->setAnchorPoint(cocos2d::Vec2(0, 0));
    _dimLayer->setPosition(_map->getOrigin());
    _worldNode->addChild(_dimLayer, 999);
}

} // namespace Controllers
