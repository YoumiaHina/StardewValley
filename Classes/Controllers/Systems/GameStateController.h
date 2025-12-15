/**
 * GameStateController: 管理时间推进与每日事件（作物生长）。
 */
#pragma once

#include "Controllers/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/CropSystem.h"

namespace Controllers {

class AnimalSystem;

class GameStateController {
public:
    GameStateController(Controllers::IMapController* map, Controllers::UIController* ui, Controllers::CropSystem* crop)
    : _map(map), _ui(ui), _crop(crop) {}

    void update(float dt);
    void sleepToNextMorning();

private:
    Controllers::IMapController* _map = nullptr;
    Controllers::UIController* _ui = nullptr;
    Controllers::CropSystem* _crop = nullptr;
    Controllers::AnimalSystem* _animals = nullptr;

public:
    void setAnimalSystem(Controllers::AnimalSystem* animals) { _animals = animals; }
};

}
