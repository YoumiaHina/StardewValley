#pragma once

#include "cocos2d.h"
#include <vector>
#include "Controllers/Map/MineMapController.h"
#include "Game/Mineral.h"

namespace Controllers {

class MineralSystem {
public:
    MineralSystem(MineMapController* map)
    : _map(map) {}

    void generateNodesForFloor(std::vector<Game::MineralData>& outNodes,
                               const std::vector<cocos2d::Vec2>& candidates,
                               const std::vector<cocos2d::Vec2>& stairWorldPos) const;

    bool hitNearestNode(std::vector<Game::MineralData>& nodes,
                        const cocos2d::Vec2& worldPos,
                        int power) const;

private:
    MineMapController* _map = nullptr;
};

}
