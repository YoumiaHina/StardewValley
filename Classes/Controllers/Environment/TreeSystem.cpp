#include "Controllers/Environment/TreeSystem.h"
#include "Game/GameConfig.h"
#include "Game/Item.h"
#include "Game/WorldState.h"
#include <random>
#include <ctime>
#include <algorithm>

using namespace cocos2d;
using namespace Game;

namespace Controllers {

namespace {
    // 随机选择树木种类（用于生成时的外观差异）。
    Game::TreeKind randomTreeKind() {
        static std::mt19937 rng{ std::random_device{}() };
        static std::uniform_int_distribution<int> dist(0, 1);
        return (dist(rng) == 0) ? Game::TreeKind::Tree1 : Game::TreeKind::Tree2;
    }
}

void TreeSystem::attachTo(Node* root) {
    _root = root;
    _cachedSeasonIndex = -1;
}

bool TreeSystem::spawnFromTile(int c, int r, const Vec2& tileCenter,
                               MapBase* map, int tileSize) {
    return spawnFromTileWithKind(c, r, tileCenter, map, tileSize, randomTreeKind());
}

bool TreeSystem::spawnFromTileWithKind(int c, int r, const Vec2& tileCenter,
                                       MapBase* map, int tileSize,
                                       Game::TreeKind kind) {
    if (!_root) return false;
    float s = static_cast<float>(tileSize);
    Vec2 footCenter = tileCenter + Vec2(0, -s * 0.5f);
    if (map && map->inBuildingArea(footCenter)) return false;
    bool blocked = map && map->collides(footCenter, 8.0f);
    if (blocked) return false;
    int seasonIndex = Game::globalState().seasonIndex;
    auto tree = Tree::create(Tree::texturePath(kind, seasonIndex));
    if (!tree) return false;
    tree->setKind(kind);
    tree->setSeasonIndex(seasonIndex);
    tree->setPosition(footCenter);
    _root->addChild(tree, 0);
    long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
    _trees[key] = tree;
    return true;
}

void TreeSystem::spawnRandom(int count, int cols, int rows,
                             const std::function<Vec2(int,int)>& tileToWorld,
                             MapBase* map, int tileSize,
                             const std::function<bool(int,int)>& isSafe) {
    if (!_root) return;
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> distC(0, cols - 1);
    std::uniform_int_distribution<int> distR(0, rows - 1);
    for (int i = 0; i < count; ++i) {
        int c = distC(rng);
        int r = distR(rng);
        if (c < 0 || r < 0 || c >= cols || r >= rows) continue;
        if (isSafe && isSafe(c,r)) continue;
        Vec2 center = tileToWorld ? tileToWorld(c, r) : Vec2::ZERO;
        float s = static_cast<float>(tileSize);
        Vec2 footCenter = center + Vec2(0, -s * 0.5f);
        if (map && map->inBuildingArea(footCenter)) continue;
        bool blocked = map && map->collides(footCenter, 8.0f);
        if (blocked) continue;
        int seasonIndex = Game::globalState().seasonIndex;
        auto kind = randomTreeKind();
        auto tree = Tree::create(Tree::texturePath(kind, seasonIndex));
        if (!tree) continue;
        tree->setKind(kind);
        tree->setSeasonIndex(seasonIndex);
        tree->setPosition(footCenter);
        _root->addChild(tree, 0);
        long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
        _trees[key] = tree;
    }
}

void TreeSystem::generateInitial(int cols, int rows,
                                 const std::function<Vec2(int,int)>& tileToWorld,
                                 MapBase* map, int tileSize,
                                 const std::function<bool(int,int)>& isBlockedTile,
                                 const std::function<bool(int,int)>& isOccupiedTile,
                                 const std::function<void(int,int)>& markOccupiedTile) {
    if (!_root) return;
    auto& ws = Game::globalState();
    if (!ws.farmTrees.empty()) return;
    int count = (cols * rows) / 40;
    if (count <= 0) return;
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> distC(0, cols - 1);
    std::uniform_int_distribution<int> distR(0, rows - 1);
    int created = 0;
    int attempts = 0;
    int maxAttempts = std::max(count * 10, 64);
    while (created < count && attempts < maxAttempts) {
        attempts++;
        int c = distC(rng);
        int r = distR(rng);
        if (c < 0 || r < 0 || c >= cols || r >= rows) continue;
        if (isBlockedTile && isBlockedTile(c, r)) continue;
        if (isOccupiedTile && isOccupiedTile(c, r)) continue;
        Vec2 center = tileToWorld ? tileToWorld(c, r) : Vec2::ZERO;
        auto kind = randomTreeKind();
        if (!spawnFromTileWithKind(c, r, center, map, tileSize, kind)) continue;
        ws.farmTrees.push_back(Game::TreePos{c, r, kind});
        if (markOccupiedTile) markOccupiedTile(c, r);
        created++;
    }
}

int TreeSystem::regrowNightlyWorldOnly(int cols, int rows,
                                      const std::function<Game::TileType(int,int)>& getTile,
                                      const std::function<bool(int,int)>& isOccupiedTile,
                                      const std::function<void(int,int)>& markOccupiedTile) {
    auto& ws = Game::globalState();
    if (cols <= 0 || rows <= 0) return 0;
    int area = cols * rows;
    int thresholdDiv = GameConfig::FARM_TREE_REGEN_THRESHOLD_DIV;
    if (thresholdDiv <= 0) thresholdDiv = 1;
    int threshold = std::max(1, area / thresholdDiv);
    if (static_cast<int>(ws.farmTrees.size()) >= threshold) return 0;

    int target = GameConfig::FARM_NIGHTLY_REGEN_COUNT;
    if (target <= 0) return 0;

    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> distC(0, cols - 1);
    std::uniform_int_distribution<int> distR(0, rows - 1);

    int created = 0;
    int attempts = 0;
    int maxAttempts = std::max(target * 30, 64);
    while (created < target && attempts < maxAttempts) {
        attempts++;
        int c = distC(rng);
        int r = distR(rng);
        if (c < 0 || r < 0 || c >= cols || r >= rows) continue;
        if (getTile && getTile(c, r) != Game::TileType::Soil) continue;
        if (isOccupiedTile && isOccupiedTile(c, r)) continue;
        auto kind = randomTreeKind();
        ws.farmTrees.push_back(Game::TreePos{c, r, kind});
        if (markOccupiedTile) markOccupiedTile(c, r);
        created++;
    }
    return created;
}

Game::Tree* TreeSystem::findTreeAt(int c, int r) const {
    long long k = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
    auto it = _trees.find(k);
    if (it != _trees.end()) return it->second;
    return nullptr;
}

bool TreeSystem::collides(const Vec2& p, float radius, int /*tileSize*/) const {
    float r2 = radius * radius;
    for (const auto& pair : _trees) {
        Game::Tree* tree = pair.second;
        if (!tree) continue;
        
        Rect rect = tree->footRect();
        float cx = std::max(rect.getMinX(), std::min(p.x, rect.getMaxX()));
        float cy = std::max(rect.getMinY(), std::min(p.y, rect.getMaxY()));
        float dx = p.x - cx;
        float dy = p.y - cy;
        if (dx*dx + dy*dy <= r2) return true;
    }
    return false;
}

bool TreeSystem::damageAt(int c, int r, int amount,
                          const std::function<void(int,int,int)>& spawnDrop,
                          const std::function<void(int,int, Game::TileType)>& setTile) {
    auto t = findTreeAt(c, r);
    if (!t) return false;
    t->applyDamage(amount);
    if (t->dead()) {
        long long k = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
        _trees.erase(k);
        {
            auto& ws = Game::globalState();
            auto& v = ws.farmTrees;
            v.erase(
                std::remove_if(v.begin(), v.end(), [c, r](const Game::TreePos& tp) {
                    return tp.c == c && tp.r == r;
                }),
                v.end()
            );
        }
        if (setTile) setTile(c, r, Game::TileType::Soil);
        t->playDestructionAnimation([t, c, r, spawnDrop]{
            t->removeFromParent();
            if (spawnDrop) spawnDrop(c, r, static_cast<int>(Game::ItemType::Wood));
        });
        return true;
    }
    return true;
}

void TreeSystem::sortTrees() {
    int seasonIndex = Game::globalState().seasonIndex;
    if (_cachedSeasonIndex != seasonIndex) {
        _cachedSeasonIndex = seasonIndex;
        for (auto& kv : _trees) {
            if (kv.second) kv.second->setSeasonIndex(seasonIndex);
        }
    }
    for (auto& kv : _trees) {
        if (kv.second) kv.second->setLocalZOrder(static_cast<int>(-kv.second->getPositionY()));
    }
}

bool TreeSystem::isEmpty() const {
    return _trees.empty();
}

std::vector<Game::TreePos> TreeSystem::getAllTreeTiles() const {
    std::vector<Game::TreePos> out;
    out.reserve(_trees.size());
    for (const auto& kv : _trees) {
        long long k = kv.first;
        int r = static_cast<int>(k >> 32);
        int c = static_cast<int>(k & 0xFFFFFFFF);
        Game::TreeKind kind = Game::TreeKind::Tree1;
        if (kv.second) kind = kv.second->kind();
        out.push_back(Game::TreePos{c, r, kind});
    }
    return out;
}

}
