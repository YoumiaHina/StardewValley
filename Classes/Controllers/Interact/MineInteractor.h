/**
 * MineInteractor: 矿洞空格交互与楼层切换、电梯激活。
 */
#pragma once

#include "cocos2d.h"
#include "Controllers/Map/MineMapController.h"
#include <functional>

namespace Controllers {

class MineInteractor {
public:
    enum class SpaceAction { None, Descend, ReturnToFarm, ReturnToEntrance, UseElevator };

    MineInteractor(Controllers::MineMapController* map,
                    std::function<cocos2d::Vec2()> getPlayerPos)
    : _map(map), _getPlayerPos(std::move(getPlayerPos)) {}

    SpaceAction onSpacePressed();

private:
    Controllers::MineMapController* _map = nullptr;
    std::function<cocos2d::Vec2()> _getPlayerPos;
};

}
// namespace Controllers
