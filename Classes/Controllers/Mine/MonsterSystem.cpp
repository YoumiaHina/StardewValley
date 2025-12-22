#include "Controllers/Mine/MonsterSystem.h"
#include "cocos2d.h"
#include "Game/SkillTree/SkillTreeSystem.h"
#include "Game/Monster/MonsterBase.h"
#include <algorithm>
#include <string>
#include <unordered_set>
#
#include "cocos2d.h"
#
using namespace cocos2d;
#
namespace Controllers {
#
namespace {
    std::mt19937& rng() {
        static std::mt19937 eng{ std::random_device{}() };
        return eng;
    }
#
    struct TileCoord {
        int c = 0;
        int r = 0;
    };
#
    bool isSlime(const Game::Monster& m) {
        using Type = Game::Monster::Type;
        return m.type == Type::GreenSlime || m.type == Type::BlueSlime || m.type == Type::RedSlime;
    }
#
    long long tileKey(int c, int r) {
        return (static_cast<long long>(c) << 32) ^ (static_cast<unsigned long long>(r) & 0xffffffffULL);
    }
#
    bool tileBlocked(const MineMapController* map, int c, int r, float radius) {
        if (!map) return true;
        if (!map->inBounds(c, r)) return true;
        Vec2 center = map->tileToWorld(c, r);
        return map->collidesWithoutMonsters(center, radius);
    }
#
    bool dfsSearch(const MineMapController* map,
                   int c, int r,
                   int tc, int tr,
                   float radius,
                   int depth,
                   int maxDepth,
                   std::unordered_set<long long>& visited,
                   std::vector<TileCoord>& out) {
        if (!map) return false;
        if (depth > maxDepth) return false;
        if (c == tc && r == tr) {
            out.push_back(TileCoord{ c, r });
            return true;
        }
        long long key = tileKey(c, r);
        if (!visited.insert(key).second) return false;
        if (tileBlocked(map, c, r, radius)) return false;
        static const int offsets[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
        for (int i = 0; i < 4; ++i) {
            int nc = c + offsets[i][0];
            int nr = r + offsets[i][1];
            if (dfsSearch(map, nc, nr, tc, tr, radius, depth + 1, maxDepth, visited, out)) {
                out.push_back(TileCoord{ c, r });
                return true;
            }
        }
        return false;
    }
#
    bool findTilePath(const MineMapController* map,
                      int sc, int sr,
                      int tc, int tr,
                      float radius,
                      int maxSteps,
                      std::vector<TileCoord>& out) {
        out.clear();
        if (!map) return false;
        std::unordered_set<long long> visited;
        if (!dfsSearch(map, sc, sr, tc, tr, radius, 0, maxSteps, visited, out)) return false;
        std::reverse(out.begin(), out.end());
        return true;
    }
#
    Game::Monster::Type randomMonsterTypeForFloor(int floor) {
        int f = floor;
        if (f < 1) f = 1;
        if (f > 50) f = 50;
        float t = 0.0f;
        if (f > 1) {
            t = static_cast<float>(f - 1) / 49.0f;
        }
        float ghost = 0.02f + 0.18f * t;
        float bug = 0.38f - 0.08f * t;
        float slime = 1.0f - ghost - bug;
        if (slime < 0.0f) slime = 0.0f;
        float greenShare = slime * 0.5f;
        float blueShare = slime * 0.3f;
        float redShare = slime - greenShare - blueShare;
        if (redShare < 0.0f) redShare = 0.0f;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float r = dist(rng());
        if (r < greenShare) return Game::Monster::Type::GreenSlime;
        if (r < greenShare + blueShare) return Game::Monster::Type::BlueSlime;
        if (r < slime) return Game::Monster::Type::RedSlime;
        if (r < slime + bug) return Game::Monster::Type::Bug;
        return Game::Monster::Type::Ghost;
    }
}

MineMonsterController::~MineMonsterController() {
    for (auto& m : _monsters) {
        if (m.sprite && m.sprite->getParent()) {
            m.sprite->removeFromParent();
        }
        m.sprite = nullptr;
    }
    if (_monsterDraw && _monsterDraw->getParent()) {
        _monsterDraw->removeFromParent();
    }
}

void MineMonsterController::generateInitialWave() {
    _monsters.clear();
    if (_map && _map->currentFloor() <= 0) { refreshVisuals(); return; }
    auto spawns = _map->monsterSpawnPoints();
    if (spawns.empty()) {
        // 没有 MonsterArea：不刷怪
        refreshVisuals();
        return;
    }
    int floor = _map ? _map->currentFloor() : 1;
    for (const auto& pt : spawns) {
        Game::Monster::Type type = randomMonsterTypeForFloor(floor);
        Monster m = Game::makeMonsterForType(type);
        m.pos = pt;
        m.textureVariant = 0;
        _monsters.push_back(m);
    }
    refreshVisuals();
}

void MineMonsterController::update(float dt) {
    if (_map && _map->currentFloor() <= 0) return; // 入口层：不刷新/重生
    // 没有 MonsterArea：不进行任何刷新/重生
    if (_map && _map->monsterSpawnPoints().empty()) return;
    _respawnAccum += dt;
    if (_respawnAccum >= 30.0f) {
        _respawnAccum = 0.0f;
        if (_monsters.size() < 8) {
            int floor = _map ? _map->currentFloor() : 1;
            Game::Monster::Type type = randomMonsterTypeForFloor(floor);
            Monster m = Game::makeMonsterForType(type);
            m.textureVariant = 0;
            _monsters.push_back(m);
        }
    }
    float tileSize = static_cast<float>(GameConfig::TILE_SIZE);
    Vec2 playerPos = _getPlayerPos ? _getPlayerPos() : Vec2::ZERO;
    float monsterRadius = tileSize * 0.4f;
    float playerRadius = tileSize * 0.4f;

    for (auto& m : _monsters) {
        if (m.attackCooldown > 0.0f) {
            m.attackCooldown = std::max(0.0f, m.attackCooldown - dt);
        }
    }

    for (auto& m : _monsters) {
        if (_getPlayerPos) {
            float range = m.searchRangeTiles * tileSize;
            Vec2 delta = playerPos - m.pos;
            float dist = delta.length();
            if (dist > 0.001f && dist <= range) {
                Vec2 dir = delta / dist;
                Vec2 proposed = m.pos + dir * m.moveSpeed * dt;
                bool blocked = false;
                if (_map && m.isCollisionAffected) {
                    if (_map->collidesWithoutMonsters(proposed, monsterRadius)) {
                        blocked = true;
                    }
                }
                if (!blocked) {
                    float toPlayer = proposed.distance(playerPos);
                    if (toPlayer < monsterRadius + playerRadius) {
                        blocked = true;
                    }
                }
                if (!blocked) {
                    m.velocity = dir * m.moveSpeed;
                    m.pos = proposed;
                } else {
                    bool movedByPath = false;
                    if (_map && isSlime(m) && m.isCollisionAffected) {
                        int sc = 0;
                        int sr = 0;
                        int tc = 0;
                        int tr = 0;
                        _map->worldToTileIndex(m.pos, sc, sr);
                        _map->worldToTileIndex(playerPos, tc, tr);
                        std::vector<TileCoord> path;
                        int maxSteps = m.searchRangeTiles * 4;
                        if (maxSteps < 8) maxSteps = 8;
                        if (findTilePath(_map, sc, sr, tc, tr, monsterRadius, maxSteps, path) && path.size() >= 2) {
                            TileCoord next = path[1];
                            Vec2 nextPos = _map->tileToWorld(next.c, next.r);
                            Vec2 dir2 = nextPos - m.pos;
                            float len2 = dir2.length();
                            if (len2 > 0.001f) {
                                Vec2 ndir = dir2 / len2;
                                Vec2 stepPos = m.pos + ndir * m.moveSpeed * dt;
                                bool blockedStep = false;
                                if (_map && m.isCollisionAffected) {
                                    if (_map->collidesWithoutMonsters(stepPos, monsterRadius)) {
                                        blockedStep = true;
                                    }
                                }
                                if (!blockedStep) {
                                    float toPlayer2 = stepPos.distance(playerPos);
                                    if (toPlayer2 < monsterRadius + playerRadius) {
                                        blockedStep = true;
                                    }
                                }
                                if (!blockedStep) {
                                    m.velocity = ndir * m.moveSpeed;
                                    m.pos = stepPos;
                                    movedByPath = true;
                                }
                            }
                        }
                    }
                    if (!movedByPath) {
                        m.velocity = Vec2::ZERO;
                    }
                }
            } else {
                m.velocity = Vec2::ZERO;
            }
        } else {
            m.velocity = Vec2::ZERO;
        }
    }

    // 怪物之间不允许重叠：简单的成对分离
    for (size_t i = 0; i < _monsters.size(); ++i) {
        for (size_t j = i + 1; j < _monsters.size(); ++j) {
            Vec2 delta = _monsters[j].pos - _monsters[i].pos;
            float dist = delta.length();
            float minDist = monsterRadius * 2.0f * 0.9f;
            if (dist > 0.0001f && dist < minDist) {
                Vec2 dir = delta / dist;
                float push = (minDist - dist) * 0.5f;
                _monsters[i].pos -= dir * push;
                _monsters[j].pos += dir * push;
            }
        }
    }
    if (_map) {
        std::vector<Rect> colliders;
        colliders.reserve(_monsters.size());
        float ts = static_cast<float>(GameConfig::TILE_SIZE);
        for (const auto& m : _monsters) {
            float half = ts * 0.5f;
            Rect rc(m.pos.x - half, m.pos.y - half, ts, ts);
            colliders.push_back(rc);
        }
        _map->setMonsterColliders(colliders);
    }
    auto& ws = Game::globalState();
    for (auto& m : _monsters) {
        float dist = m.pos.distance(playerPos);
        float attackRange = monsterRadius + playerRadius + 2.0f;
        if (dist < attackRange && m.attackCooldown <= 0.0f && m.dmg > 0) {
            ws.hp = std::max(0, ws.hp - m.dmg);
            m.attackCooldown = 0.8f;
        }
    }
    if (ws.hp <= 0) {
        for (auto& m : _monsters) {
            if (m.sprite && m.sprite->getParent()) {
                m.sprite->removeFromParent();
            }
            m.sprite = nullptr;
        }
        _monsters.clear();
        _respawnAccum = 0.0f;
        if (_map) {
            std::vector<Rect> colliders;
            _map->setMonsterColliders(colliders);
        }
        if (_monsterDraw) {
            _monsterDraw->clear();
        }
        return;
    }
    refreshVisuals();
}

void MineMonsterController::resetFloor() {
    for (auto& m : _monsters) {
        if (m.sprite && m.sprite->getParent()) {
            m.sprite->removeFromParent();
        }
        m.sprite = nullptr;
    }
    _monsters.clear();
    _respawnAccum = 0.0f;
    if (_map) {
        std::vector<Rect> colliders;
        _map->setMonsterColliders(colliders);
    }
    refreshVisuals();
}

void MineMonsterController::applyDamageAt(const Vec2& worldPos, int baseDamage) {
    // 简化：对距离最近的一只怪物造成伤害
    int idx = -1; float best = 1e9f;
    for (int i=0;i<(int)_monsters.size();++i) {
        float d = _monsters[i].pos.distance(worldPos);
        if (d < best) { best = d; idx = i; }
    }
    if (idx >= 0) {
        Monster m = _monsters[idx];
        int def = m.def;
        int dmg = std::max(0, baseDamage - def);
        m.hp -= dmg;
        if (m.hp <= 0) {
            auto& ws = Game::globalState();
            auto& skill = Game::SkillTreeSystem::getInstance();
            long long baseGold = 10;
            long long reward = skill.adjustGoldRewardForCombat(baseGold);
            ws.gold += reward;
            skill.addXp(Game::SkillTreeType::Combat, skill.xpForCombatKill(baseGold));
            auto drops = m.getDrops();
            if (_map) {
                int c = 0;
                int r = 0;
                _map->worldToTileIndex(m.pos, c, r);
                for (auto t : drops) {
                    _map->spawnDropAt(c, r, static_cast<int>(t), 1);
                }
            }
            cocos2d::Sprite* sprite = m.sprite;
            if (sprite) {
                const auto& info = Game::monsterInfoFor(m.type);
                info.playDeathAnimation(m, sprite, [sprite]() {
                    if (sprite->getParent()) {
                        sprite->removeFromParent();
                    }
                });
            }
            _monsters.erase(_monsters.begin() + idx);
        } else {
            _monsters[idx].hp = m.hp;
        }
    }
}

void MineMonsterController::applyAreaDamage(const std::vector<std::pair<int,int>>& tiles, int baseDamage) {
    if (!_map || tiles.empty()) return;
    auto matches = [this,&tiles](const Monster& m) {
        int c = 0, r = 0;
        _map->worldToTileIndex(m.pos, c, r);
        for (const auto& t : tiles) {
            if (c == t.first && r == t.second) return true;
        }
        return false;
    };
    for (std::size_t i = 0; i < _monsters.size();) {
        Monster m = _monsters[i];
        if (!matches(m)) { ++i; continue; }
        int dmg = std::max(0, baseDamage - m.def);
        m.hp -= dmg;
        if (m.hp <= 0) {
            auto& ws = Game::globalState();
            auto& skill = Game::SkillTreeSystem::getInstance();
            long long baseGold = 10;
            long long reward = skill.adjustGoldRewardForCombat(baseGold);
            ws.gold += reward;
            skill.addXp(Game::SkillTreeType::Combat, skill.xpForCombatKill(baseGold));
            auto drops = m.getDrops();
            if (_map) {
                int c = 0;
                int r = 0;
                _map->worldToTileIndex(m.pos, c, r);
                for (auto t : drops) {
                    _map->spawnDropAt(c, r, static_cast<int>(t), 1);
                }
            }
            cocos2d::Sprite* sprite = m.sprite;
            if (sprite) {
                const auto& info = Game::monsterInfoFor(m.type);
                info.playDeathAnimation(m, sprite, [sprite]() {
                    if (sprite->getParent()) {
                        sprite->removeFromParent();
                    }
                });
            }
            _monsters.erase(_monsters.begin() + static_cast<long>(i));
            continue;
        }
        _monsters[i].hp = m.hp;
        ++i;
    }
}

void MineMonsterController::refreshVisuals() {
    if (!_monsterDraw) {
        _monsterDraw = DrawNode::create();
        if (_worldNode) _worldNode->addChild(_monsterDraw, 4);
    }
    _monsterDraw->clear();
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    for (auto& m : _monsters) {
        if (!m.sprite && _worldNode) {
            std::string path = Game::monsterTexturePath(m);
            auto spr = Sprite::create(path);
            if (spr) {
                spr->setAnchorPoint(Vec2(0.5f, 0.0f));
                spr->setPosition(m.pos);
                _worldNode->addChild(spr, 3);
                m.sprite = spr;
            }
        }
        if (m.sprite) {
            m.sprite->setPosition(m.pos);
            const auto& info = Game::monsterInfoFor(m.type);
            cocos2d::Vec2 v = m.velocity;
            float len2 = v.x * v.x + v.y * v.y;
            if (len2 > 1e-4f) {
                info.playMoveAnimation(m, m.sprite);
            } else {
                info.playStaticAnimation(m, m.sprite);
            }
        }
    }
}

} // namespace Controllers
