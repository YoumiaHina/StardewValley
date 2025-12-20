#include "Controllers/Environment/WeedSystem.h"
#include "Game/GameConfig.h"
#include "Game/Item.h"
#include "Game/WorldState.h"
#include <random>
#include <ctime>
#include <algorithm>

using namespace cocos2d;
using namespace Game;

namespace Controllers {

void WeedSystem::attachTo(Node* root) {
    _root = root;
}

bool WeedSystem::spawnFromTile(int c, int r, const Vec2& tileCenter,
                               MapBase* map, int tileSize) {
    if (!_root) return false;
    float s = static_cast<float>(tileSize);
    Vec2 footCenter = tileCenter + Vec2(0, -s * 0.5f);
    if (map && (map->inBuildingArea(footCenter) || map->inWallArea(footCenter))) return false;
    bool blocked = map && map->collides(footCenter, 8.0f);
    if (blocked) return false;
    auto weed = Weed::create("FarmEnvironment/grass.png");
    if (!weed) return false;
    weed->setBrokenTexture("FarmEnvironment/grass_broken.png");
    weed->setPosition(footCenter);
    _root->addChild(weed, 0);
    long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
    _weeds[key] = weed;
    return true;
}

void WeedSystem::spawnRandom(int count, int cols, int rows,
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
        auto weed = Weed::create("FarmEnvironment/grass.png");
        if (!weed) continue;
        weed->setBrokenTexture("FarmEnvironment/grass_broken.png");
        weed->setPosition(footCenter);
        _root->addChild(weed, 0);
        long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
        _weeds[key] = weed;
    }
}

void WeedSystem::generateInitial(int cols, int rows,
                                 const std::function<Vec2(int,int)>& tileToWorld,
                                 MapBase* map, int tileSize,
                                 const std::function<bool(int,int)>& isBlockedTile,
                                 const std::function<bool(int,int)>& isOccupiedTile,
                                 const std::function<void(int,int)>& markOccupiedTile) {
    if (!_root) return;
    auto& ws = Game::globalState();
    if (!ws.farmWeeds.empty()) return;
    int count = (cols * rows) / 35;
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
        if (!spawnFromTile(c, r, center, map, tileSize)) continue;
        ws.farmWeeds.push_back(Game::WeedPos{c, r});
        if (markOccupiedTile) markOccupiedTile(c, r);
        created++;
    }
}

int WeedSystem::regrowNightlyWorldOnly(int cols, int rows,
                                       const std::function<Game::TileType(int,int)>& getTile,
                                       const std::function<bool(int,int)>& isOccupiedTile,
                                       const std::function<void(int,int)>& markOccupiedTile) {
    auto& ws = Game::globalState();
    if (cols <= 0 || rows <= 0) return 0;
    int area = cols * rows;
    int thresholdDiv = GameConfig::FARM_WEED_REGEN_THRESHOLD_DIV;
    if (thresholdDiv <= 0) thresholdDiv = 1;
    int threshold = std::max(1, area / thresholdDiv);
    if (static_cast<int>(ws.farmWeeds.size()) >= threshold) return 0;

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
        ws.farmWeeds.push_back(Game::WeedPos{c, r});
        if (markOccupiedTile) markOccupiedTile(c, r);
        created++;
    }
    return created;
}

Game::Weed* WeedSystem::findWeedAt(int c, int r) const {
    long long k = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
    auto it = _weeds.find(k);
    if (it != _weeds.end()) return it->second;
    return nullptr;
}

bool WeedSystem::collides(const Vec2& p, float radius, int) const {
    float r2 = radius * radius;
    for (const auto& pair : _weeds) {
        Game::Weed* weed = pair.second;
        if (!weed) continue;
        Rect rect = weed->footRect();
        float cx = std::max(rect.getMinX(), std::min(p.x, rect.getMaxX()));
        float cy = std::max(rect.getMinY(), std::min(p.y, rect.getMaxY()));
        float dx = p.x - cx;
        float dy = p.y - cy;
        if (dx*dx + dy*dy <= r2) return true;
    }
    return false;
}

bool WeedSystem::damageAt(int c, int r, int amount,
                          const std::function<void(int,int,int)>& spawnDrop,
                          const std::function<void(int,int, Game::TileType)>&) {
    long long k = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
    auto it = _weeds.find(k);
    if (it == _weeds.end()) return false;
    auto weed = it->second;
    if (!weed) return false;
    weed->applyDamage(amount);
    if (weed->dead()) {
        _weeds.erase(k);
        {
            auto& ws = Game::globalState();
            auto& v = ws.farmWeeds;
            v.erase(
                std::remove_if(v.begin(), v.end(), [c, r](const Game::WeedPos& wp) {
                    return wp.c == c && wp.r == r;
                }),
                v.end()
            );
        }
        weed->playDestructionAnimation([weed, c, r, spawnDrop]{
            if (spawnDrop) spawnDrop(c, r, static_cast<int>(Game::ItemType::Fiber));
            weed->removeFromParent();
        });
        return true;
    }
    return true;
}

bool WeedSystem::isEmpty() const {
    return _weeds.empty();
}

void WeedSystem::sortWeeds() {
    for (auto& kv : _weeds) {
        if (kv.second) kv.second->setLocalZOrder(static_cast<int>(-kv.second->getPositionY()));
    }
}

std::vector<Game::WeedPos> WeedSystem::getAllWeedTiles() const {
    std::vector<Game::WeedPos> out;
    out.reserve(_weeds.size());
    for (const auto& kv : _weeds) {
        long long k = kv.first;
        int r = static_cast<int>(k >> 32);
        int c = static_cast<int>(k & 0xFFFFFFFF);
        Game::WeedPos wp;
        wp.c = c;
        wp.r = r;
        out.push_back(wp);
    }
    return out;
}

}

