#pragma once

#include "cocos2d.h"
#include "Controllers/Map/TownMapController.h"
#include <functional>

namespace Controllers {

class TownInteractor {
public:
    enum class SpaceAction { None, EnterFarm };

    void setMap(TownMapController* m) { _map = m; }
    void setGetPlayerPos(std::function<cocos2d::Vec2()> f) { _getPlayerPos = std::move(f); }

    SpaceAction onSpacePressed();

private:
    Controllers::TownMapController* _map = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
};

} // namespace Controllers
