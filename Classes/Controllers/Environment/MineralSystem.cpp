#include "Controllers/Environment/MineralSystem.h"
#include "Controllers/Map/MineMapController.h"
#include "Game/GameConfig.h"
#include "Game/EnvironmentObstacle/Mineral.h"
#include <random>
#include <algorithm>

using namespace cocos2d;

namespace Controllers {

void MineralSystem::attachTo(Node* root) {
    _root = root;
}

void MineralSystem::clearVisuals() {
    for (auto& kv : _obstacles) {
        if (kv.second) {
            kv.second->removeFromParent();
        }
    }
    _obstacles.clear();
}

void MineralSystem::syncVisuals() {
    if (!_root || !_runtime) return;
    clearVisuals();
    for (const auto& data : *_runtime) {
        auto mineral = Game::Mineral::create(data.texture);
        if (!mineral) continue;
        mineral->setType(data.type);
        mineral->setBrokenTexture("FarmEnvironment/rock_broken.png");
        mineral->setPosition(data.pos);
        _root->addChild(mineral, 0);
        int c = 0;
        int r = 0;
        if (_map) {
            _map->worldToTileIndex(data.pos, c, r);
        }
        long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
        _obstacles[key] = mineral;
    }
}

void MineralSystem::generateNodesForFloor(std::vector<Game::MineralData>& outNodes,
                                          const std::vector<Vec2>& candidates,
                                          const std::vector<Vec2>& stairWorldPos) const {
    outNodes.clear();
    if (!_map || _map->currentFloor() <= 0) return;

    std::mt19937 rng{ std::random_device{}() };
    int floor = _map->currentFloor();
    bool allowCopper = floor >= 1;
    bool allowIron   = floor >= 5;
    bool allowGold   = floor >= 10;
    std::uniform_real_distribution<float> prob(0.0f, 1.0f);

    std::vector<Vec2> selected = candidates;

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

    auto alignToTileCenter = [this](const Vec2& wp){
        int c = 0;
        int r = 0;
        _map->worldToTileIndex(wp, c, r);
        return _map->tileToWorld(c, r);
    };

    std::uniform_real_distribution<float> probRock(0.0f,1.0f);
    std::uniform_int_distribution<int> normalIdx(1,5);
    std::uniform_int_distribution<int> hardIdx(1,3);
    float spawnChance = 0.3f;
    for (const auto& rawP : selected) {
        Vec2 p = alignToTileCenter(rawP);
        if (isStairTile(p)) {
            continue;
        }
        if (prob(rng) > spawnChance) {
            continue;
        }
        bool placed = false;
        float oreRoll = prob(rng);
        if (allowGold && oreRoll < 0.01f) {
            Game::MineralData m;
            m.type = Game::MineralType::GoldOre;
            m.hp = 2;
            m.sizeTiles = 1;
            m.pos = p;
            m.texture = "Mineral/goldOre.png";
            outNodes.push_back(m);
            placed = true;
        } else if (allowIron && oreRoll < 0.03f) {
            Game::MineralData m;
            m.type = Game::MineralType::IronOre;
            m.hp = 2;
            m.sizeTiles = 1;
            m.pos = p;
            m.texture = "Mineral/ironOre.png";
            outNodes.push_back(m);
            placed = true;
        } else if (allowCopper && oreRoll < 0.05f) {
            Game::MineralData m;
            m.type = Game::MineralType::CopperOre;
            m.hp = 2;
            m.sizeTiles = 1;
            m.pos = p;
            m.texture = "Mineral/copperOre.png";
            outNodes.push_back(m);
            placed = true;
        }
        if (placed) {
            continue;
        }
        float r = probRock(rng);
        if (r < 0.08f) {
            Game::MineralData m;
            m.type = Game::MineralType::HugeRock;
            m.hp = 5;
            m.sizeTiles = 2;
            m.pos = p;
            m.texture = "Rock/hugeRock.png";
            outNodes.push_back(m);
        } else if (r < 0.25f) {
            int hi = hardIdx(rng);
            Game::MineralData m;
            m.type = Game::MineralType::HardRock;
            m.hp = 3;
            m.sizeTiles = 1;
            m.pos = p;
            if (hi == 1) m.texture = "Rock/hardRock1.png";
            else if (hi == 2) m.texture = "Rock/hardRock2.png";
            else m.texture = "Rock/hardRock3.png";
            outNodes.push_back(m);
        } else {
            int ni = normalIdx(rng);
            Game::MineralData m;
            m.type = Game::MineralType::Rock;
            m.hp = 1;
            m.sizeTiles = 1;
            m.pos = p;
            switch (ni) {
                case 1: m.texture = "Rock/Rock1.png"; break;
                case 2: m.texture = "Rock/Rock2.png"; break;
                case 3: m.texture = "Rock/Rock3.png"; break;
                case 4: m.texture = "Rock/Rock4.png"; break;
                default: m.texture = "Rock/Rock5.png"; break;
            }
            outNodes.push_back(m);
        }
    }

    for (const auto& stairPos : stairWorldPos) {
        if (prob(rng) > 0.8f) {
            continue;
        }
        Vec2 p = alignToTileCenter(stairPos);
        float r = probRock(rng);
        if (r < 0.08f) {
            Game::MineralData m;
            m.type = Game::MineralType::HugeRock;
            m.hp = 5;
            m.sizeTiles = 2;
            m.pos = p;
            m.texture = "Rock/hugeRock.png";
            outNodes.push_back(m);
        } else if (r < 0.25f) {
            int hi = hardIdx(rng);
            Game::MineralData m;
            m.type = Game::MineralType::HardRock;
            m.hp = 3;
            m.sizeTiles = 1;
            m.pos = p;
            if (hi == 1) m.texture = "Rock/hardRock1.png";
            else if (hi == 2) m.texture = "Rock/hardRock2.png";
            else m.texture = "Rock/hardRock3.png";
            outNodes.push_back(m);
        } else {
            int ni = normalIdx(rng);
            Game::MineralData m;
            m.type = Game::MineralType::Rock;
            m.hp = 1;
            m.sizeTiles = 1;
            m.pos = p;
            switch (ni) {
                case 1: m.texture = "Rock/Rock1.png"; break;
                case 2: m.texture = "Rock/Rock2.png"; break;
                case 3: m.texture = "Rock/Rock3.png"; break;
                case 4: m.texture = "Rock/Rock4.png"; break;
                default: m.texture = "Rock/Rock5.png"; break;
            }
            outNodes.push_back(m);
        }
    }
}

bool MineralSystem::hitNearestNode(std::vector<Game::MineralData>& nodes,
                                   const Vec2& worldPos,
                                   int power) const {
    int idx = -1;
    float best = 1e9f;
    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        float d = nodes[i].pos.distance(worldPos);
        if (d < best) {
            best = d;
            idx = i;
        }
    }
    float ts = static_cast<float>(GameConfig::TILE_SIZE);
    if (idx >= 0 && best <= ts * 0.6f) {
        nodes[idx].hp -= power;
        if (nodes[idx].hp <= 0) {
            nodes.erase(nodes.begin() + idx);
            return true;
        }
        return true;
    }
    return false;
}

bool MineralSystem::spawnFromTile(int, int, const Vec2&,
                                  Game::MapBase*, int) {
    return false;
}

void MineralSystem::spawnRandom(int, int, int,
                                const std::function<Vec2(int,int)>&,
                                Game::MapBase*, int,
                                const std::function<bool(int,int)>&) {
}

bool MineralSystem::collides(const Vec2& p, float radius, int) const {
    float r2 = radius * radius;
    for (const auto& kv : _obstacles) {
        Game::Mineral* mineral = kv.second;
        if (!mineral) continue;
        Rect rect = mineral->footRect();
        float cx = std::max(rect.getMinX(), std::min(p.x, rect.getMaxX()));
        float cy = std::max(rect.getMinY(), std::min(p.y, rect.getMaxY()));
        float dx = p.x - cx;
        float dy = p.y - cy;
        if (dx * dx + dy * dy <= r2) return true;
    }
    return false;
}

bool MineralSystem::damageAt(int c, int r, int amount,
                             const std::function<void(int,int,int)>& spawnDrop,
                             const std::function<void(int,int, Game::TileType)>& setTile) {
    if (!_map || !_runtime || _runtime->empty()) return false;
    Vec2 center = _map->tileToWorld(c, r);
    int idx = -1;
    float best = 1e9f;
    for (int i = 0; i < static_cast<int>(_runtime->size()); ++i) {
        float d = (*_runtime)[i].pos.distance(center);
        if (d < best) {
            best = d;
            idx = i;
        }
    }
    float ts = static_cast<float>(GameConfig::TILE_SIZE);
    if (idx < 0 || best > ts * 0.6f) {
        return false;
    }
    auto data = (*_runtime)[idx];
    long long key = 0;
    if (_map) {
        int mc = 0;
        int mr = 0;
        _map->worldToTileIndex(data.pos, mc, mr);
        key = (static_cast<long long>(mr) << 32) | static_cast<unsigned long long>(mc);
    }
    Game::Mineral* mineral = nullptr;
    if (key != 0) {
        auto it = _obstacles.find(key);
        if (it != _obstacles.end()) {
            mineral = it->second;
        }
    }
    if (mineral) {
        mineral->applyDamage(amount);
    }
    data.hp -= amount;
    if (data.hp <= 0) {
        if (setTile) setTile(c, r, Game::TileType::Soil);
        Game::MineralType type = data.type;
        if (mineral) {
            long long eraseKey = key;
            mineral->playDestructionAnimation([this, mineral, eraseKey, c, r, spawnDrop, type]() {
                if (spawnDrop) {
                    Game::ItemType drop = Game::mineralDropItem(type);
                    spawnDrop(c, r, static_cast<int>(drop));
                }
                auto it2 = _obstacles.find(eraseKey);
                if (it2 != _obstacles.end() && it2->second == mineral) {
                    _obstacles.erase(it2);
                }
                mineral->removeFromParent();
            });
        } else {
            if (spawnDrop) {
                Game::ItemType drop = Game::mineralDropItem(type);
                spawnDrop(c, r, static_cast<int>(drop));
            }
        }
        _runtime->erase(_runtime->begin() + idx);
        return true;
    }
    (*_runtime)[idx] = data;
    return true;
}

bool MineralSystem::isEmpty() const {
    if (!_runtime) return true;
    return _runtime->empty();
}

}
