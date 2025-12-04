#include "Controllers/AbyssMiningController.h"
#include "cocos2d.h"
#include <random>

using namespace cocos2d;

namespace Controllers {

void AbyssMiningController::generateNodesForFloor() {
    _nodes.clear();
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> countDist(15, 30);
    int n = countDist(rng);
    std::uniform_real_distribution<float> distX(0.0f, _map->getContentSize().width);
    std::uniform_real_distribution<float> distY(0.0f, _map->getContentSize().height);
    for (int i=0;i<n;++i) {
        Node node; node.type = NodeType::Rock; node.hp = 3; // 简化为通用岩石
        node.pos = Vec2(distX(rng), distY(rng));
        _nodes.push_back(node);
    }
    refreshVisuals();
}

bool AbyssMiningController::hitNearestNode(const Vec2& worldPos, int power) {
    int idx = -1; float best = 1e9f;
    for (int i=0;i<(int)_nodes.size();++i) {
        float d = _nodes[i].pos.distance(worldPos);
        if (d < best) { best = d; idx = i; }
    }
    if (idx >= 0) {
        _nodes[idx].hp -= power;
        if (_nodes[idx].hp <= 0) {
            // 简化：掉落石头 1 个
            if (auto inv = Game::globalState().inventory) inv->addItems(Game::ItemType::Stone, 1);
            _nodes.erase(_nodes.begin() + idx);
            refreshVisuals();
            return true;
        }
    }
    return false;
}

void AbyssMiningController::refreshVisuals() {
    if (!_miningDraw) {
        _miningDraw = DrawNode::create();
        if (_worldNode) _worldNode->addChild(_miningDraw, 2);
    }
    _miningDraw->clear();
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    for (const auto& n : _nodes) {
        Color4F col = Color4F(0.5f,0.5f,0.5f,1.0f);
        switch (n.type) {
            case NodeType::Rock: col = Color4F(0.55f,0.55f,0.60f,1.0f); break;
            case NodeType::Copper: col = Color4F(0.85f,0.45f,0.25f,1.0f); break;
            case NodeType::Iron: col = Color4F(0.65f,0.65f,0.70f,1.0f); break;
            case NodeType::Coal: col = Color4F(0.20f,0.20f,0.20f,1.0f); break;
            case NodeType::Emerald: col = Color4F(0.25f,0.80f,0.45f,1.0f); break;
            case NodeType::Aquamarine: col = Color4F(0.20f,0.65f,0.85f,1.0f); break;
            case NodeType::Diamond: col = Color4F(0.90f,0.95f,1.0f,1.0f); break;
            case NodeType::Gold: col = Color4F(0.90f,0.80f,0.25f,1.0f); break;
            case NodeType::Iridium: col = Color4F(0.65f,0.25f,0.85f,1.0f); break;
            case NodeType::Ruby: col = Color4F(0.85f,0.25f,0.25f,1.0f); break;
            case NodeType::Mystery: col = Color4F(0.75f,0.75f,0.85f,1.0f); break;
            case NodeType::PrismShard: col = Color4F(1.0f,0.85f,0.95f,1.0f); break;
        }
        Vec2 c = n.pos;
        Vec2 a(c.x - s*0.30f, c.y - s*0.30f);
        Vec2 b(c.x + s*0.30f, c.y - s*0.30f);
        Vec2 d(c.x - s*0.30f, c.y + s*0.30f);
        Vec2 e(c.x + s*0.30f, c.y + s*0.30f);
        Vec2 rect[4] = { a,b,e,d };
        _miningDraw->drawSolidPoly(rect, 4, col);
    }
}

} // namespace Controllers