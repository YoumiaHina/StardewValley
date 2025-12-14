#pragma once

#include "cocos2d.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/NPC/NpcControllerBase.h"
#include <functional>

namespace Controllers {

class TownInteractor {
public:
    enum class SpaceAction { None, EnterFarm };

    void setMap(TownMapController* m) { _map = m; }
    void setGetPlayerPos(std::function<cocos2d::Vec2()> f) { _getPlayerPos = std::move(f); }
    void setNpcController(TownNpcController* c) { _npc = c; }
    void setUI(Controllers::UIController* ui) { _ui = ui; }
    void setInventory(std::shared_ptr<Game::Inventory> inv) { _inventory = std::move(inv); }

    SpaceAction onSpacePressed();

private:
    Controllers::TownMapController* _map = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
    Controllers::TownNpcController* _npc = nullptr;
    Controllers::UIController* _ui = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
};

} // namespace Controllers
