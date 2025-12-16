#include "Controllers/Environment/TreeSystem.h"
#include "Game/GameConfig.h"
#include "Game/Item.h"
#include <random>
#include <ctime>
#include <algorithm>

using namespace cocos2d;
using namespace Game;

namespace Controllers {

void TreeSystem::attachTo(Node* root) {
    _root = root;
}

bool TreeSystem::spawnFromTile(int c, int r, const Vec2& tileCenter,
                               IMapBase* map, int tileSize) {
    if (!_root) return false;
    float s = static_cast<float>(tileSize);
    Vec2 footCenter = tileCenter + Vec2(0, -s * 0.5f);
    bool blocked = map && map->collides(footCenter, 8.0f);
    if (blocked) return false;
    auto tree = Tree::create("Tree/tree.png");
    if (!tree) return false;
    tree->setPosition(footCenter);
    _root->addChild(tree, 0);
    long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
    _trees[key] = tree;
    return true;
}

void TreeSystem::spawnRandom(int count, int cols, int rows,
                             const std::function<Vec2(int,int)>& tileToWorld,
                             IMapBase* map, int tileSize,
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
        auto tree = Tree::create("Tree/tree.png");
        if (!tree) continue;
        tree->setPosition(footCenter);
        _root->addChild(tree, 0);
        long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
        _trees[key] = tree;
    }
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
        out.push_back(Game::TreePos{c, r});
    }
    return out;
}

}
