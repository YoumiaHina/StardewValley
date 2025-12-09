/**
 * GameStateController: 管理时间推进与每日事件（作物生长）。
 */
#pragma once

#include "Controllers/IMapController.h"
#include "Controllers/UIController.h"
#include "Controllers/CropSystem.h"

namespace Controllers {

class GameStateController {
public:
    GameStateController(IMapController* map, UIController* ui, CropSystem* crop)
    : _map(map), _ui(ui), _crop(crop) {}

    void update(float dt);

private:
    IMapController* _map = nullptr;
    UIController* _ui = nullptr;
    CropSystem* _crop = nullptr;
};

} // namespace Controllers
