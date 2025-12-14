#include "Controllers/MineMiningController.h"
#include "cocos2d.h"
#include <random>

using namespace cocos2d;

namespace Controllers {

void MineMiningController::resetFloor() {
    for (auto &n : _nodes) {
        if (n.sprite) { n.sprite->removeFromParent(); n.sprite = nullptr; }
    }
    _nodes.clear();
    for (auto &s : _stairs) {
        if (s.sprite) { s.sprite->removeFromParent(); s.sprite = nullptr; }
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
    if (_map && _map->currentFloor() <= 0) { _nodes.clear(); _stairs.clear(); refreshVisuals(); return; }
    for (auto &n : _nodes) { if (n.sprite) { n.sprite->removeFromParent(); n.sprite = nullptr; } }
    for (auto &s : _stairs) { if (s.sprite) { s.sprite->removeFromParent(); s.sprite = nullptr; } }
    _nodes.clear();
    _stairs.clear();
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
    int floor = _map->currentFloor();
    bool allowCopper = floor >= 1;
    bool allowIron   = floor >= 20;
    bool allowGold   = floor >= 40;
    std::uniform_real_distribution<float> prob(0.0f, 1.0f);
    std::uniform_real_distribution<float> sel(0.0f, 1.0f);

    int targetCount = static_cast<int>(candidates.size() * 0.30f);
    targetCount = std::max(0, std::min(targetCount, 300));
    std::shuffle(candidates.begin(), candidates.end(), rng);
    std::vector<Vec2> selected;
    selected.assign(candidates.begin(), candidates.begin() + targetCount);

    std::vector<Vec2> remaining;
    remaining.reserve(selected.size());
    auto alignToTileCenter = [this](const Vec2& wp){ int c=0,r=0; _map->worldToTileIndex(wp,c,r); return _map->tileToWorld(c,r); };

    std::vector<Vec2> stairWorldPos;
    if (!selected.empty()) {
        int maxStairs = std::min<int>(5, static_cast<int>(selected.size()));
        int minStairs = std::min<int>(2, maxStairs);
        if (maxStairs > 0 && minStairs > 0) {
            std::uniform_int_distribution<int> stairCountDist(minStairs, maxStairs);
            int stairCount = stairCountDist(rng);
            stairCount = std::max(0, std::min(stairCount, static_cast<int>(selected.size())));
            for (int i = 0; i < stairCount; ++i) {
                Vec2 p = alignToTileCenter(selected[i]);
                stairWorldPos.push_back(p);
            }
        }
    }

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
    for (const auto& rawP : selected) {
        Vec2 p = alignToTileCenter(rawP);
        if (isStairTile(p)) {
            continue;
        }
        bool placedOre = false;
        if (allowGold && prob(rng) < 0.06f) {
            Node n; n.type = NodeType::GoldOre; n.hp = 2; n.sizeTiles = 1; n.pos = p; n.tex = "Mineral/goldOre.png"; _nodes.push_back(n); placedOre = true;
        } else if (allowIron && prob(rng) < 0.12f) {
            Node n; n.type = NodeType::IronOre; n.hp = 2; n.sizeTiles = 1; n.pos = p; n.tex = "Mineral/ironOre.png"; _nodes.push_back(n); placedOre = true;
        } else if (allowCopper && prob(rng) < 0.18f) {
            Node n; n.type = NodeType::CopperOre; n.hp = 2; n.sizeTiles = 1; n.pos = p; n.tex = "Mineral/copperOre.png"; _nodes.push_back(n); placedOre = true;
        }
        if (!placedOre) remaining.push_back(p);
    }
    // 填充普通石头 / 硬石 / 巨石（简化：不做全局距离检查，避免 O(n^2) 卡顿）
    std::uniform_real_distribution<float> probRock(0.0f,1.0f);
    std::uniform_int_distribution<int> normalIdx(1,5);
    std::uniform_int_distribution<int> hardIdx(1,3);
    for (const auto& rawP : remaining) {
        Vec2 p = alignToTileCenter(rawP);
        if (isStairTile(p)) {
            continue;
        }
        float r = probRock(rng);
        if (r < 0.08f) {
            // 巨石：2x2，5 次破坏
            Node n; n.type = NodeType::HugeRock; n.hp = 5; n.sizeTiles = 2; n.pos = p; n.tex = "Rock/hugeRock.png"; _nodes.push_back(n);
        } else if (r < 0.25f) {
            // 硬石：3 次破坏
            int hi = hardIdx(rng);
            Node n; n.type = NodeType::HardRock; n.hp = 3; n.sizeTiles = 1; n.pos = p; n.tex = (hi==1?"Rock/hardRock1.png":(hi==2?"Rock/hardRock2.png":"Rock/hardRock3.png")); _nodes.push_back(n);
        } else {
            // 普通石头：1 次破坏
            int ni = normalIdx(rng);
            Node n; n.type = NodeType::Rock; n.hp = 1; n.sizeTiles = 1; n.pos = p;
            switch (ni){
                case 1: n.tex = "Rock/Rock1.png"; break;
                case 2: n.tex = "Rock/Rock2.png"; break;
                case 3: n.tex = "Rock/Rock3.png"; break;
                case 4: n.tex = "Rock/Rock4.png"; break;
                default: n.tex = "Rock/Rock5.png"; break;
            }
            _nodes.push_back(n);
        }
    }

    // 为每个楼梯格强制生成一块普通石头，初始时阻塞下楼
    for (const auto& p : stairWorldPos) {
        Node n;
        n.type = NodeType::Rock;
        n.hp = 1;
        n.sizeTiles = 1;
        n.pos = p;
        n.tex = "Rock/Rock1.png";
        _nodes.push_back(n);
    }
    std::vector<Rect> colliders; colliders.reserve(_nodes.size() + _stairs.size());
    float ts = static_cast<float>(GameConfig::TILE_SIZE);
    for (const auto& n : _nodes) {
        float half = ts * 0.5f * n.sizeTiles;
        Rect rc(n.pos.x - half, n.pos.y - half, ts * n.sizeTiles, ts * n.sizeTiles);
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
    int idx = -1; float best = 1e9f;
    for (int i=0;i<(int)_nodes.size();++i) {
        float d = _nodes[i].pos.distance(worldPos);
        if (d < best) { best = d; idx = i; }
    }
    float ts = static_cast<float>(GameConfig::TILE_SIZE);
    if (idx >= 0 && best <= ts * 0.6f) {
        _nodes[idx].hp -= power;
        if (_nodes[idx].hp <= 0) {
            // 掉落：矿石掉对应矿物，石头掉 Stone
            if (auto inv = Game::globalState().inventory) {
                switch (_nodes[idx].type) {
                    case NodeType::CopperOre: inv->addItems(Game::ItemType::CopperGrain, 1); break;
                    case NodeType::IronOre:   inv->addItems(Game::ItemType::IronGrain, 1); break;
                    case NodeType::GoldOre:   inv->addItems(Game::ItemType::GoldGrain, 1); break;
                    default: inv->addItems(Game::ItemType::Stone, 1); break;
                }
            }
            if (_nodes[idx].sprite) { _nodes[idx].sprite->removeFromParent(); _nodes[idx].sprite = nullptr; }
            _nodes.erase(_nodes.begin() + idx);
            // 更新碰撞与可视
            if (_map) {
                std::vector<Rect> colliders; colliders.reserve(_nodes.size() + _stairs.size());
                float ts = static_cast<float>(GameConfig::TILE_SIZE);
                for (const auto& n2 : _nodes) {
                    float half2 = ts * 0.5f * n2.sizeTiles;
                    Rect rc2(n2.pos.x - half2, n2.pos.y - half2, ts * n2.sizeTiles, ts * n2.sizeTiles);
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
            refreshVisuals();
            return true; // destroyed
        }
        return true; // partial hit
    }
    return false;
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
            _map->worldToTileIndex(n.pos, nc, nr);
            if (nc == sc && nr == sr) {
                covered = true;
                break;
            }
        }
        if (st.sprite) {
            st.sprite->setVisible(!covered);
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
    }
    if (_miningDraw) {
        _miningDraw->clear();
    }
    for (auto &n : _nodes) {
        if (!n.sprite) {
            auto sp = Sprite::create();
            sp->setTexture(n.tex); // 路径示例：Mineral/copperOre.png、Rock/Rock1.png
            if (!sp->getTexture()) continue; // 路径错误则跳过
            auto cs = sp->getContentSize();
            float targetW = s * n.sizeTiles;
            float targetH = s * n.sizeTiles;
            float sx = (cs.width > 0) ? (targetW / cs.width) : 1.0f;
            float sy = (cs.height > 0) ? (targetH / cs.height) : 1.0f;
            sp->setScale(std::min(sx, sy));
            sp->setPosition(n.pos);
            if (_worldNode) _worldNode->addChild(sp, 3);
            n.sprite = sp;
        } else {
            n.sprite->setPosition(n.pos);
        }
    }
    for (auto &st : _stairs) {
        if (!st.sprite) {
            auto sp = Sprite::create("Maps/mine/stair.png");
            if (!sp->getTexture()) continue;
            auto cs = sp->getContentSize();
            float targetW = s;
            float targetH = s;
            float sx = (cs.width > 0) ? (targetW / cs.width) : 1.0f;
            float sy = (cs.height > 0) ? (targetH / cs.height) : 1.0f;
            sp->setScale(std::min(sx, sy));
            sp->setPosition(st.pos);
            if (_worldNode) _worldNode->addChild(sp, 1);
            st.sprite = sp;
        } else {
            st.sprite->setPosition(st.pos);
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
