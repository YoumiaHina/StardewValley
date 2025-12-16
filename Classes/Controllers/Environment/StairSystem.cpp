#include "Controllers/Environment/StairSystem.h"
#include "Controllers/Map/MineMapController.h"
#include "Game/GameConfig.h"
#include <random>
#include <algorithm>

using namespace cocos2d;

namespace Controllers {

void StairSystem::attachTo(Node* root) {
    _root = root;
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
    return true;
}

void StairSystem::generateStairs(const std::vector<Vec2>& candidates,
                                 int minCount,
                                 int maxCount,
                                 std::vector<Vec2>& outWorldPos) const {
    outWorldPos.clear();
    if (!_map || candidates.empty()) return;
    int maxStairs = std::min(maxCount, static_cast<int>(candidates.size()));
    int minStairsClamped = std::min(minCount, maxStairs);
    if (maxStairs <= 0 || minStairsClamped <= 0) return;
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> stairCountDist(minStairsClamped, maxStairs);
    int stairCount = stairCountDist(rng);
    stairCount = std::max(0, std::min(stairCount, static_cast<int>(candidates.size())));
    auto alignToTileCenter = [this](const Vec2& wp){
        int c = 0;
        int r = 0;
        _map->worldToTileIndex(wp, c, r);
        return _map->tileToWorld(c, r);
    };
    for (int i = 0; i < stairCount; ++i) {
        Vec2 p = alignToTileCenter(candidates[i]);
        outWorldPos.push_back(p);
    }
}

}

