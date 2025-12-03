/**
 * GameStateController: 管理时间推进与每日事件（作物生长）。
 */
#pragma once

#include "Controllers/IMapController.h"
#include "Controllers/UIController.h"

namespace Controllers {

class GameStateController {
public:
    GameStateController(IMapController* map, UIController* ui)
    : _map(map), _ui(ui) {}

    void update(float dt);

private:
    IMapController* _map = nullptr;
    UIController* _ui = nullptr;
};

} // namespace Controllers