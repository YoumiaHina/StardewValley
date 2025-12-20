#include "Controllers/Environment/RockSystem.h"
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
    // 随机选择石头种类（用于生成时的外观差异）。
    Game::RockKind randomRockKind() {
        static std::mt19937 rng{ std::random_device{}() };
        static std::uniform_int_distribution<int> dist(0, 1);
        return (dist(rng) == 0) ? Game::RockKind::Rock1 : Game::RockKind::Rock2;
    }
}

void RockSystem::attachTo(Node* root) {
    _root = root;
}

bool RockSystem::spawnFromTile(int c, int r, const Vec2& tileCenter,
                               MapBase* map, int tileSize) {
    return spawnFromTileWithKind(c, r, tileCenter, map, tileSize, randomRockKind());
}

bool RockSystem::spawnFromTileWithKind(int c, int r, const Vec2& tileCenter,
                                       MapBase* map, int tileSize,
                                       Game::RockKind kind) {
    if (!_root) return false;
    float s = static_cast<float>(tileSize);
    Vec2 footCenter = tileCenter + Vec2(0, -s * 0.5f);
    if (map && (map->inBuildingArea(footCenter) || map->inWallArea(footCenter))) return false;
    bool blocked = map && map->collides(footCenter, 8.0f);
    if (blocked) return false;
    auto rock = Rock::create(Rock::texturePath(kind));
    if (!rock) return false;
    rock->setKind(kind);
    rock->setBrokenTexture("FarmEnvironment/rock_broken.png");
    rock->setPosition(footCenter);
    _root->addChild(rock, 0);
    long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
    _rocks[key] = rock;
    return true;
}

void RockSystem::spawnRandom(int count, int cols, int rows,
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
        bool blocked = map && map->collides(footCenter, 8.0f);
        if (blocked) continue;
        auto kind = randomRockKind();
        auto rock = Rock::create(Rock::texturePath(kind));
        if (!rock) continue;
        rock->setKind(kind);
        rock->setBrokenTexture("FarmEnvironment/rock_broken.png");
        rock->setPosition(footCenter);
        _root->addChild(rock, 0);
        long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
        _rocks[key] = rock;
    }
}

void RockSystem::generateInitial(int cols, int rows,
                                 const std::function<Vec2(int,int)>& tileToWorld,
                                 MapBase* map, int tileSize,
                                 const std::function<bool(int,int)>& isBlockedTile,
                                 const std::function<bool(int,int)>& isOccupiedTile,
                                 const std::function<void(int,int)>& markOccupiedTile) {
    if (!_root) return;
    auto& ws = Game::globalState();
    if (!ws.farmRocks.empty()) return;
    int count = (cols * rows) / 22;
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
        auto kind = randomRockKind();
        if (!spawnFromTileWithKind(c, r, center, map, tileSize, kind)) continue;
        ws.farmRocks.push_back(Game::RockPos{c, r, kind});
        if (markOccupiedTile) markOccupiedTile(c, r);
        created++;
    }
}

int RockSystem::regrowNightlyWorldOnly(int cols, int rows,
                                       const std::function<Game::TileType(int,int)>& getTile,
                                       const std::function<bool(int,int)>& isOccupiedTile,
                                       const std::function<void(int,int)>& markOccupiedTile) {
    auto& ws = Game::globalState();
    if (cols <= 0 || rows <= 0) return 0;
    int area = cols * rows;
    int thresholdDiv = GameConfig::FARM_ROCK_REGEN_THRESHOLD_DIV;
    if (thresholdDiv <= 0) thresholdDiv = 1;
    int threshold = std::max(1, area / thresholdDiv);
    if (static_cast<int>(ws.farmRocks.size()) >= threshold) return 0;

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
        auto kind = randomRockKind();
        ws.farmRocks.push_back(Game::RockPos{c, r, kind});
        if (markOccupiedTile) markOccupiedTile(c, r);
        created++;
    }
    return created;
}

Game::Rock* RockSystem::findRockAt(int c, int r) const {
    long long k = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
    auto it = _rocks.find(k);
    if (it != _rocks.end()) return it->second;
    return nullptr;
}

bool RockSystem::collides(const Vec2& p, float radius, int) const {
    float r2 = radius * radius;
    for (const auto& pair : _rocks) {
        Game::Rock* rock = pair.second;
        if (!rock) continue;
        Rect rect = rock->footRect();
        float cx = std::max(rect.getMinX(), std::min(p.x, rect.getMaxX()));
        float cy = std::max(rect.getMinY(), std::min(p.y, rect.getMaxY()));
        float dx = p.x - cx;
        float dy = p.y - cy;
        if (dx*dx + dy*dy <= r2) return true;
    }
    return false;
}

bool RockSystem::damageAt(int c, int r, int amount,
                          const std::function<void(int,int,int)>& spawnDrop,
                          const std::function<void(int,int, Game::TileType)>& setTile) {
    auto rock = findRockAt(c, r);
    if (!rock) return false;
    rock->applyDamage(amount);
    if (rock->dead()) {
        long long k = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
        _rocks.erase(k);
        {
            auto& ws = Game::globalState();
            auto& v = ws.farmRocks;
            v.erase(
                std::remove_if(v.begin(), v.end(), [c, r](const Game::RockPos& rp) {
                    return rp.c == c && rp.r == r;
                }),
                v.end()
            );
        }
        if (setTile) setTile(c, r, Game::TileType::Soil);
        rock->playDestructionAnimation([rock, c, r, spawnDrop]{
            if (spawnDrop) spawnDrop(c, r, static_cast<int>(Game::ItemType::Stone));
            rock->removeFromParent();
        });
        return true;
    }
    return true;
}

void RockSystem::sortRocks() {
    for (auto& kv : _rocks) {
        if (kv.second) kv.second->setLocalZOrder(static_cast<int>(-kv.second->getPositionY()));
    }
}

bool RockSystem::isEmpty() const {
    return _rocks.empty();
}

std::vector<Game::RockPos> RockSystem::getAllRockTiles() const {
    std::vector<Game::RockPos> out;
    out.reserve(_rocks.size());
    for (const auto& kv : _rocks) {
        long long k = kv.first;
        int r = static_cast<int>(k >> 32);
        int c = static_cast<int>(k & 0xFFFFFFFF);
        Game::RockKind kind = Game::RockKind::Rock1;
        if (kv.second) kind = kv.second->kind();
        out.push_back(Game::RockPos{c, r, kind});
    }
    return out;
}

}
