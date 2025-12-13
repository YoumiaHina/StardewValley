#pragma once

#include "Controllers/UI/UIController.h"
#include "Controllers/Map/BeachMapController.h"
#include "Game/Inventory.h"

namespace Controllers {

class BeachInteractor {
public:
    enum class SpaceAction { None, EnterFarm };

    void setMap(BeachMapController* m) { _map = m; }
    void setInventory(Game::Inventory* inv) { _inventory = inv; }
    void setUI(UIController* ui) { _ui = ui; }
    void setGetPlayerPos(std::function<cocos2d::Vec2()> f) { _getPlayerPos = std::move(f); }

    SpaceAction onSpacePressed();

private:
    BeachMapController* _map = nullptr;
    Game::Inventory* _inventory = nullptr;
    UIController* _ui = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
};

} // namespace Controllers

