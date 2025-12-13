#include "Controllers/AbyssMiningController.h"
#include "cocos2d.h"
#include <random>

using namespace cocos2d;

namespace Controllers {

void AbyssMiningController::resetFloor() {
    _nodes.clear();
    refreshVisuals();
}

void AbyssMiningController::generateNodesForFloor() {
    if (_map && _map->currentFloor() <= 0) { _nodes.clear(); refreshVisuals(); return; }
    _nodes.clear();
    auto rects = _map->rockAreaRects();
    auto polys = _map->rockAreaPolys();
    auto addNodeAt = [this](const Vec2& p){ Node node; node.type = NodeType::Rock; node.hp = 3; node.pos = p; _nodes.push_back(node); };
    // Sample rects with grid spacing ~0.8 tile
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float step = s * 0.8f;
    for (const auto& r : rects) {
        for (float y = r.getMinY() + step*0.5f; y <= r.getMaxY() - step*0.5f; y += step) {
            for (float x = r.getMinX() + step*0.5f; x <= r.getMaxX() - step*0.5f; x += step) {
                addNodeAt(Vec2(x, y));
            }
        }
    }
    // Sample polys using rejection within bounding box
    for (const auto& poly : polys) {
        if (poly.size() < 3) continue;
        float minX = poly[0].x, maxX = poly[0].x, minY = poly[0].y, maxY = poly[0].y;
        for (const auto& p : poly) { minX = std::min(minX, p.x); maxX = std::max(maxX, p.x); minY = std::min(minY, p.y); maxY = std::max(maxY, p.y); }
        std::mt19937 rng{ std::random_device{}() };
        std::uniform_real_distribution<float> distX(minX, maxX);
        std::uniform_real_distribution<float> distY(minY, maxY);
        auto inside = [&poly](const Vec2& pt){
            bool in = false; size_t j = poly.size() - 1;
            for (size_t i=0;i<poly.size();++i){
                if (((poly[i].y > pt.y) != (poly[j].y > pt.y)) && (pt.x < (poly[j].x - poly[i].x) * (pt.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x)) in = !in;
                j = i;
            }
            return in;
        };
        int samples = 40; // reasonable fill
        for (int k=0;k<samples;++k) {
            Vec2 pt(distX(rng), distY(rng)); if (inside(pt)) addNodeAt(pt);
        }
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