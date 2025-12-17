#include "Controllers/Environment/StairSystem.h"
#include "Controllers/Map/MineMapController.h"
#include "Game/GameConfig.h"
#include <random>
#include <algorithm>

using namespace cocos2d;

namespace Controllers {

void StairSystem::attachTo(Node* root) {
    _root = root;
    if (_debugDraw) {
        _debugDraw->removeFromParent();
        _debugDraw = nullptr;
    }
    if (_root) {
        _debugDraw = DrawNode::create();
        _root->addChild(_debugDraw, 2);
    }
}

void StairSystem::reset() {
    for (auto& s : _stairs) {
        if (s.node) {
            s.node->removeFromParent();
            s.node = nullptr;
        }
    }
    _stairs.clear();
    if (_debugDraw) {
        _debugDraw->clear();
    }
}

bool StairSystem::spawnFromTile(int, int, const Vec2&,
                               Game::MapBase*, int) {
    return false;
}

void StairSystem::spawnRandom(int, int, int,
                              const std::function<Vec2(int,int)>&,
                              Game::MapBase*, int,
                              const std::function<bool(int,int)>&) {
}

bool StairSystem::collides(const Vec2&, float, int) const {
    return false;
}

bool StairSystem::damageAt(int, int, int,
                           const std::function<void(int,int,int)>&,
                           const std::function<void(int,int, Game::TileType)>&) {
    return false;
}

bool StairSystem::isEmpty() const {
    return _stairs.empty();
}

void StairSystem::generateStairs(const std::vector<Vec2>& candidates,
                                 int minCount,
                                 int maxCount,
                                 std::vector<Vec2>& outWorldPos) {
    outWorldPos.clear();
    reset();
    if (!_map || candidates.empty()) return;
    int maxStairs = std::min(maxCount, static_cast<int>(candidates.size()));
    int minStairsClamped = std::min(minCount, maxStairs);
    if (maxStairs <= 0 || minStairsClamped <= 0) return;
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> stairCountDist(minStairsClamped, maxStairs);
    int stairCount = stairCountDist(rng);
    stairCount = std::max(0, std::min(stairCount, static_cast<int>(candidates.size())));
    std::vector<Vec2> shuffled = candidates;
    std::shuffle(shuffled.begin(), shuffled.end(), rng);
    auto alignToTileCenter = [this](const Vec2& wp){
        int c = 0;
        int r = 0;
        _map->worldToTileIndex(wp, c, r);
        return _map->tileToWorld(c, r);
    };
    for (int i = 0; i < stairCount; ++i) {
        Vec2 p = alignToTileCenter(shuffled[i]);
        outWorldPos.push_back(p);
        StairData data;
        data.pos = p;
        data.node = nullptr;
        data.covered = false;
        _stairs.push_back(data);
    }
}

void StairSystem::syncExtraStairsToMap(const std::vector<Game::MineralData>& minerals) {
    if (!_map) return;
    std::vector<Vec2> usable;
    usable.reserve(_stairs.size());
    for (auto& st : _stairs) {
        int sc = 0;
        int sr = 0;
        _map->worldToTileIndex(st.pos, sc, sr);
        bool covered = false;
        for (const auto& m : minerals) {
            int mc = 0;
            int mr = 0;
            _map->worldToTileIndex(m.pos, mc, mr);
            if (mc == sc && mr == sr) {
                covered = true;
                break;
            }
        }
        st.covered = covered;
        if (st.node) {
            st.node->setVisible(!covered);
        }
        if (!covered) {
            usable.push_back(st.pos);
        }
    }
    if (_map) {
        _map->setExtraStairs(usable);
    }
    if (_debugDraw) {
        _debugDraw->clear();
        float s = static_cast<float>(GameConfig::TILE_SIZE);
        for (const auto& st : _stairs) {
            float half = s * 0.5f;
            Vec2 a(st.pos.x - half, st.pos.y - half);
            Vec2 b(st.pos.x + half, st.pos.y - half);
            Vec2 c(st.pos.x + half, st.pos.y + half);
            Vec2 d(st.pos.x - half, st.pos.y + half);
            Color4F col(0.0f, 1.0f, 0.0f, 0.4f);
            _debugDraw->drawLine(a, b, col);
            _debugDraw->drawLine(b, c, col);
            _debugDraw->drawLine(c, d, col);
            _debugDraw->drawLine(d, a, col);
        }
    }
}

void StairSystem::refreshVisuals() {
    if (!_root) return;
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    for (auto& st : _stairs) {
        if (!st.node) {
            auto stairNode = Game::Stair::create("Maps/mine/stair.png");
            if (!stairNode) continue;
            Size spriteSize = stairNode->spriteContentSize();
            float targetW = s;
            float targetH = s;
            float sx = spriteSize.width > 0 ? targetW / spriteSize.width : 1.0f;
            float sy = spriteSize.height > 0 ? targetH / spriteSize.height : 1.0f;
            float scale = std::min(sx, sy);
            stairNode->setScale(scale);
            stairNode->setPosition(st.pos);
            _root->addChild(stairNode, 1);
            st.node = stairNode;
        } else {
            st.node->setPosition(st.pos);
        }
        if (st.node) {
            st.node->setVisible(!st.covered);
        }
    }
}

}
