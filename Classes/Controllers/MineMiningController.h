/**
 * MineMiningController: 矿洞采矿系统（分布、挖掘规则、特殊元素）。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include <string>
#include "Controllers/Map/MineMapController.h"
#include "Game/WorldState.h"
#include "Game/Item.h"
#include "Game/Mineral.h"

namespace Controllers {

class MineMiningController {
public:
    struct Node {
        Game::MineralData mineral;
        Game::Mineral* node = nullptr;
    };

    struct Stair {
        cocos2d::Vec2 pos;
        cocos2d::Sprite* sprite = nullptr;
    };

    MineMiningController(MineMapController* map, cocos2d::Node* worldNode)
    : _map(map), _worldNode(worldNode) {}

    void generateNodesForFloor();
    void update(float dt) {}
    // 清空当前楼层的采矿点并刷新可视
    void resetFloor();
    // 命中采矿点：返回是否破坏成功并进行掉落
    bool hitNearestNode(const cocos2d::Vec2& worldPos, int power);
    void refreshVisuals();

private:
    void syncExtraStairsToMap();

    MineMapController* _map = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    std::vector<Node> _nodes;
    std::vector<Stair> _stairs;
    cocos2d::DrawNode* _miningDraw = nullptr; // legacy, kept for cleanup
};

} // namespace Controllers
