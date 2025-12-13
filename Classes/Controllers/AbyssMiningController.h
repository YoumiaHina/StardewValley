/**
 * AbyssMiningController: 深渊矿洞采矿系统（分布、挖掘规则、特殊元素）。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include "Controllers/Map/AbyssMapController.h"
#include "Game/WorldState.h"
#include "Game/Item.h"

namespace Controllers {

class AbyssMiningController {
public:
    enum class NodeType { Rock, Copper, Iron, Coal, Emerald, Aquamarine, Diamond, Gold, Iridium, Ruby, Mystery, PrismShard };

    struct Node { NodeType type; cocos2d::Vec2 pos; int hp; };

    AbyssMiningController(AbyssMapController* map, cocos2d::Node* worldNode)
    : _map(map), _worldNode(worldNode) {}

    void generateNodesForFloor();
    void update(float dt) {}
    // 清空当前楼层的采矿点并刷新可视
    void resetFloor();
    // 命中采矿点：返回是否破坏成功并进行掉落
    bool hitNearestNode(const cocos2d::Vec2& worldPos, int power);
    void refreshVisuals();

private:
    AbyssMapController* _map = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    std::vector<Node> _nodes;
    cocos2d::DrawNode* _miningDraw = nullptr;
};

} // namespace Controllers