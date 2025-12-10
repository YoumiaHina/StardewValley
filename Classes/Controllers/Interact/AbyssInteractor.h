/**
 * AbyssInteractor: 深渊矿洞空格交互与楼层切换、电梯激活。
 */
#pragma once

#include "cocos2d.h"
#include "Controllers/Map/AbyssMapController.h"
#include <functional>

namespace Controllers {

class AbyssInteractor {
public:
    enum class SpaceAction { None, Descend };

    AbyssInteractor(Controllers::AbyssMapController* map,
                    std::function<cocos2d::Vec2()> getPlayerPos)
    : _map(map), _getPlayerPos(std::move(getPlayerPos)) {}

    SpaceAction onSpacePressed();

private:
    Controllers::AbyssMapController* _map = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
};

}
// namespace Controllers
