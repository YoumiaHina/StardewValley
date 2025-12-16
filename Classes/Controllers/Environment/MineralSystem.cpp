#include "Controllers/Environment/MineralSystem.h"
#include "Game/GameConfig.h"
#include "Game/Mineral.h"
#include <random>
#include <algorithm>

using namespace cocos2d;

namespace Controllers {

void MineralSystem::generateNodesForFloor(std::vector<Game::MineralData>& outNodes,
                                          const std::vector<Vec2>& candidates,
                                          const std::vector<Vec2>& stairWorldPos) const {
    outNodes.clear();
    if (!_map || _map->currentFloor() <= 0) return;

    std::mt19937 rng{ std::random_device{}() };
    int floor = _map->currentFloor();
    bool allowCopper = floor >= 1;
    bool allowIron   = floor >= 20;
    bool allowGold   = floor >= 40;
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

    std::vector<Vec2> remaining;
    remaining.reserve(selected.size());
    for (const auto& rawP : selected) {
        Vec2 p = alignToTileCenter(rawP);
        if (isStairTile(p)) {
            continue;
        }
        bool placedOre = false;
        if (allowGold && prob(rng) < 0.06f) {
            Game::MineralData m;
            m.type = Game::MineralType::GoldOre;
            m.hp = 2;
            m.sizeTiles = 1;
            m.pos = p;
            m.texture = "Mineral/goldOre.png";
            outNodes.push_back(m);
            placedOre = true;
        } else if (allowIron && prob(rng) < 0.12f) {
            Game::MineralData m;
            m.type = Game::MineralType::IronOre;
            m.hp = 2;
            m.sizeTiles = 1;
            m.pos = p;
            m.texture = "Mineral/ironOre.png";
            outNodes.push_back(m);
            placedOre = true;
        } else if (allowCopper && prob(rng) < 0.18f) {
            Game::MineralData m;
            m.type = Game::MineralType::CopperOre;
            m.hp = 2;
            m.sizeTiles = 1;
            m.pos = p;
            m.texture = "Mineral/copperOre.png";
            outNodes.push_back(m);
            placedOre = true;
        }
        if (!placedOre) remaining.push_back(p);
    }

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

}
