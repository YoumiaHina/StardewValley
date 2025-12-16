#include "Controllers/MineMiningController.h"
#include "cocos2d.h"
#include <random>
using namespace cocos2d;

namespace Controllers {

void MineMiningController::resetFloor() {
    for (auto &n : _nodes) {
        if (n.node) { n.node->removeFromParent(); n.node = nullptr; }
    }
    _nodes.clear();
    _minerals.clear();
    for (auto &s : _stairs) {
        if (s.node) { s.node->removeFromParent(); s.node = nullptr; }
    }
    _stairs.clear();
    if (_map) {
        std::vector<cocos2d::Rect> empty;
        _map->setDynamicColliders(empty);
        std::vector<Vec2> noStairs;
        _map->setExtraStairs(noStairs);
    }
    refreshVisuals();
}

void MineMiningController::generateNodesForFloor() {
    if (_map && _map->currentFloor() <= 0) { _nodes.clear(); _stairs.clear(); _minerals.clear(); refreshVisuals(); return; }
    for (auto &n : _nodes) { if (n.node) { n.node->removeFromParent(); n.node = nullptr; } }
    for (auto &s : _stairs) { if (s.node) { s.node->removeFromParent(); s.node = nullptr; } }
    _nodes.clear();
    _stairs.clear();
    _minerals.clear();
    auto* mineralSystem = _map ? _map->mineralSystem() : nullptr;
    if (mineralSystem) mineralSystem->bindRuntimeStorage(&_minerals);
    auto rects = _map->rockAreaRects();
    auto polys = _map->rockAreaPolys();
    std::vector<Vec2> candidates;
    candidates.reserve(256);
    // Sample rects with grid spacing ~0.8 tile
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float step = s * 0.8f;
    for (const auto& r : rects) {
        for (float y = r.getMinY() + step*0.5f; y <= r.getMaxY() - step*0.5f; y += step) {
            for (float x = r.getMinX() + step*0.5f; x <= r.getMaxX() - step*0.5f; x += step) {
                candidates.emplace_back(x, y);
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
            Vec2 pt(distX(rng), distY(rng)); if (inside(pt)) candidates.push_back(pt);
        }
    }
    std::mt19937 rng{ std::random_device{}() };
    int targetCount = static_cast<int>(candidates.size() * 0.30f);
    targetCount = std::max(0, std::min(targetCount, 300));
    std::shuffle(candidates.begin(), candidates.end(), rng);
    std::vector<Vec2> selected;
    selected.assign(candidates.begin(), candidates.begin() + targetCount);

    auto alignToTileCenter = [this](const Vec2& wp){ int c=0,r=0; _map->worldToTileIndex(wp,c,r); return _map->tileToWorld(c,r); };

    std::vector<Vec2> stairWorldPos;
    _stairSystem.generateStairs(selected, 2, 5, stairWorldPos);

    std::vector<std::pair<int,int>> stairTiles;
    stairTiles.reserve(stairWorldPos.size());
    for (const auto& p : stairWorldPos) {
        int c = 0;
        int r = 0;
        _map->worldToTileIndex(p, c, r);
        stairTiles.emplace_back(c, r);
    }
    auto isStairTile = [this,&stairTiles](const Vec2& wp){
        int c = 0;
        int r = 0;
        _map->worldToTileIndex(wp, c, r);
        for (const auto& t : stairTiles) {
            if (t.first == c && t.second == r) {
                return true;
            }
        }
        return false;
    };

    for (const auto& p : stairWorldPos) {
        Stair sNode;
        sNode.pos = p;
        _stairs.push_back(sNode);
    }

    if (mineralSystem) mineralSystem->generateNodesForFloor(_minerals, selected, stairWorldPos);
    for (const auto& m : _minerals) {
        Node n;
        n.mineral = m;
        _nodes.push_back(n);
    }

    for (const auto& p : stairWorldPos) {
        Game::MineralData m;
        m.type = Game::MineralType::Rock;
        m.hp = 1;
        m.sizeTiles = 1;
        m.pos = p;
        m.texture = "Rock/Rock1.png";
        _minerals.push_back(m);
        Node n;
        n.mineral = m;
        _nodes.push_back(n);
    }
    std::vector<Rect> colliders;
    colliders.reserve(_nodes.size() + _stairs.size());
    float ts = static_cast<float>(GameConfig::TILE_SIZE);
    for (const auto& n : _nodes) {
        float half = ts * 0.5f * n.mineral.sizeTiles;
        Rect rc(n.mineral.pos.x - half, n.mineral.pos.y - half, ts * n.mineral.sizeTiles, ts * n.mineral.sizeTiles);
        colliders.push_back(rc);
    }
    for (const auto& s : _stairs) {
        float half = ts * 0.5f;
        Rect rc(s.pos.x - half, s.pos.y - half, ts, ts);
        colliders.push_back(rc);
    }
    _map->setDynamicColliders(colliders);
    refreshVisuals();
    syncExtraStairsToMap();
}

bool MineMiningController::hitNearestNode(const Vec2& worldPos, int power) {
    if (!_map) return false;
    auto* mineralSystem = _map->mineralSystem();
    if (!mineralSystem) return false;
    mineralSystem->bindRuntimeStorage(&_minerals);
    int idx = -1;
    float best = 1e9f;
    for (int i = 0; i < static_cast<int>(_minerals.size()); ++i) {
        float d = _minerals[i].pos.distance(worldPos);
        if (d < best) {
            best = d;
            idx = i;
        }
    }
    float ts = static_cast<float>(GameConfig::TILE_SIZE);
    if (idx < 0 || best > ts * 0.6f) {
        return false;
    }

    int tc = 0;
    int tr = 0;
    _map->worldToTileIndex(_minerals[idx].pos, tc, tr);
    int hpBefore = _minerals[idx].hp;

    auto spawnDrop = [](int, int, int item) {
        auto inv = Game::globalState().inventory;
        if (inv) {
            inv->addItems(static_cast<Game::ItemType>(item), 1);
        }
    };

    auto setTileNoop = [](int, int, Game::TileType) {};

    bool hit = mineralSystem->damageAt(tc, tr, power, spawnDrop, setTileNoop);
    if (!hit) {
        return false;
    }

    bool destroyed = (hpBefore - power) <= 0;

    if (!destroyed) {
        if (idx >= 0 && idx < static_cast<int>(_nodes.size())) {
            _nodes[idx].mineral.hp = hpBefore - power;
            if (_nodes[idx].node) {
                _nodes[idx].node->applyDamage(power);
            }
        }
        return true;
    }

    Game::Mineral* deadNode = nullptr;
    if (idx >= 0 && idx < static_cast<int>(_nodes.size())) {
        deadNode = _nodes[idx].node;
        _nodes.erase(_nodes.begin() + idx);
    }
    if (idx >= 0 && idx < static_cast<int>(_minerals.size())) {
        _minerals.erase(_minerals.begin() + idx);
    }

    if (_map) {
        std::vector<Rect> colliders;
        colliders.reserve(_nodes.size() + _stairs.size());
        for (const auto& n2 : _nodes) {
            float half2 = ts * 0.5f * n2.mineral.sizeTiles;
            Rect rc2(n2.mineral.pos.x - half2, n2.mineral.pos.y - half2, ts * n2.mineral.sizeTiles, ts * n2.mineral.sizeTiles);
            colliders.push_back(rc2);
        }
        for (const auto& s : _stairs) {
            float half = ts * 0.5f;
            Rect rc(s.pos.x - half, s.pos.y - half, ts, ts);
            colliders.push_back(rc);
        }
        _map->setDynamicColliders(colliders);
    }
    syncExtraStairsToMap();
    if (deadNode) {
        deadNode->playDestructionAnimation([deadNode](){
            deadNode->removeFromParent();
        });
    }
    refreshVisuals();
    return true;
}

void MineMiningController::syncExtraStairsToMap() {
    if (!_map) return;
    std::vector<Vec2> usable;
    usable.reserve(_stairs.size());
    for (auto& st : _stairs) {
        int sc = 0;
        int sr = 0;
        _map->worldToTileIndex(st.pos, sc, sr);
        bool covered = false;
        for (const auto& n : _nodes) {
            int nc = 0;
            int nr = 0;
            _map->worldToTileIndex(n.mineral.pos, nc, nr);
            if (nc == sc && nr == sr) {
                covered = true;
                break;
            }
        }
        if (st.node) {
            st.node->setVisible(!covered);
        }
        if (!covered) {
            usable.push_back(st.pos);
        }
    }
    _map->setExtraStairs(usable);
}

void MineMiningController::refreshVisuals() {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    if (!_miningDraw && _worldNode) {
        _miningDraw = DrawNode::create();
        _worldNode->addChild(_miningDraw, 50);
    } else if (_miningDraw && !_miningDraw->getParent() && _worldNode) {
        _miningDraw->removeFromParent();
        _miningDraw = DrawNode::create();
        _worldNode->addChild(_miningDraw, 50);
    }
    if (_miningDraw) {
        _miningDraw->clear();
    }
    for (auto &n : _nodes) {
        if (!n.node) {
            auto mineralNode = Game::Mineral::create(n.mineral.texture);
            if (!mineralNode) continue;
            auto cs = mineralNode->spriteContentSize();
            float targetW = s * n.mineral.sizeTiles;
            float targetH = s * n.mineral.sizeTiles;
            float sx = (cs.width > 0) ? (targetW / cs.width) : 1.0f;
            float sy = (cs.height > 0) ? (targetH / cs.height) : 1.0f;
            mineralNode->setScale(std::min(sx, sy));
            mineralNode->setPosition(n.mineral.pos);
            if (_worldNode) _worldNode->addChild(mineralNode, 3);
            n.node = mineralNode;
        } else {
            n.node->setPosition(n.mineral.pos);
        }
    }
    for (auto &st : _stairs) {
        if (!st.node) {
            auto stairNode = Game::Stair::create("Maps/mine/stair.png");
            if (!stairNode) continue;
            auto spriteSize = stairNode->spriteContentSize();
            float targetW = s;
            float targetH = s;
            float sx = (spriteSize.width > 0) ? (targetW / spriteSize.width) : 1.0f;
            float sy = (spriteSize.height > 0) ? (targetH / spriteSize.height) : 1.0f;
            stairNode->setScale(std::min(sx, sy));
            stairNode->setPosition(st.pos);
            if (_worldNode) _worldNode->addChild(stairNode, 1);
            st.node = stairNode;
        } else {
            st.node->setPosition(st.pos);
        }
        if (_miningDraw) {
            float half = s * 0.5f;
            Vec2 a(st.pos.x - half, st.pos.y - half);
            Vec2 b(st.pos.x + half, st.pos.y - half);
            Vec2 c(st.pos.x + half, st.pos.y + half);
            Vec2 d(st.pos.x - half, st.pos.y + half);
            Color4F col(0.0f, 1.0f, 0.0f, 0.4f);
            _miningDraw->drawLine(a, b, col);
            _miningDraw->drawLine(b, c, col);
            _miningDraw->drawLine(c, d, col);
            _miningDraw->drawLine(d, a, col);
        }
    }
}

} // namespace Controllers
