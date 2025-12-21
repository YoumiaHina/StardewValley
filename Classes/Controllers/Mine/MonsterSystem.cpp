#include "Controllers/Mine/MonsterSystem.h"
#include "cocos2d.h"
#include "Game/SkillTree/SkillTreeSystem.h"
#include <algorithm>
#include <string>

using namespace cocos2d;

namespace Controllers {

namespace {
    std::mt19937& rng() {
        static std::mt19937 eng{ std::random_device{}() };
        return eng;
    }

    int randomSlimeVariantForFloor(int floor) {
        int f = floor;
        if (f < 1) f = 1;
        if (f > 50) f = 50;
        float t = 0.0f;
        if (f > 1) {
            t = static_cast<float>(f - 1) / 49.0f;
        }
        float green = 0.5f * (1.0f - t);
        float blue = 0.3f;
        float red = 1.0f - green - blue;
        if (red < 0.0f) red = 0.0f;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float r = dist(rng());
        if (r < green) return 0;
        if (r < green + blue) return 1;
        return 2;
    }

    void applySlimeStatsForVariant(Game::Monster& m, int variant) {
        int baseHp = m.maxHp > 0 ? m.maxHp : m.hp;
        int baseDmg = m.dmg;
        float hpMul = 1.0f;
        float dmgMul = 1.0f;
        if (variant == 2) {
            hpMul = 5.0f;
            dmgMul = 5.0f;
            m.name = "Red Slime";
        } else if (variant == 1) {
            hpMul = 3.0f;
            dmgMul = 3.0f;
            m.name = "Blue Slime";
        } else {
            m.name = "Green Slime";
        }
        m.hp = static_cast<int>(baseHp * hpMul);
        m.maxHp = m.hp;
        m.dmg = static_cast<int>(baseDmg * dmgMul);
    }

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
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float r = dist(rng());
        if (r < slime) return Game::Monster::Type::RockSlime;
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
        if (type == Game::Monster::Type::RockSlime) {
            int variant = randomSlimeVariantForFloor(floor);
            m.textureVariant = variant;
            applySlimeStatsForVariant(m, variant);
        } else {
            m.textureVariant = 0;
        }
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
            if (type == Game::Monster::Type::RockSlime) {
                int variant = randomSlimeVariantForFloor(floor);
                m.textureVariant = variant;
                applySlimeStatsForVariant(m, variant);
            } else {
                m.textureVariant = 0;
            }
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

    // 怪物朝玩家移动，带基础碰撞：不能穿过墙体/矿石/玩家
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
                    m.velocity = Vec2::ZERO;
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
        int def = _monsters[idx].def;
        int dmg = std::max(0, baseDamage - def);
        _monsters[idx].hp -= dmg;
        if (_monsters[idx].hp <= 0) {
            auto& m = _monsters[idx];
            {
                auto& ws = Game::globalState();
                auto& skill = Game::SkillTreeSystem::getInstance();
                long long baseGold = 10;
                long long reward = skill.adjustGoldRewardForCombat(baseGold);
                ws.gold += reward;
                skill.addXp(Game::SkillTreeType::Combat, skill.xpForCombatKill(baseGold));
            }
            auto drops = m.getDrops();
            if (_map) {
                int c = 0;
                int r = 0;
                _map->worldToTileIndex(m.pos, c, r);
                for (auto t : drops) {
                    _map->spawnDropAt(c, r, static_cast<int>(t), 1);
                }
            }
            if (m.sprite && m.sprite->getParent()) {
                m.sprite->removeFromParent();
            }
            m.sprite = nullptr;
            _monsters.erase(_monsters.begin() + idx);
            refreshVisuals();
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
        auto& m = _monsters[i];
        if (!matches(m)) { ++i; continue; }
        int dmg = std::max(0, baseDamage - m.def);
        m.hp -= dmg;
        if (m.hp <= 0) {
            {
                auto& ws = Game::globalState();
                auto& skill = Game::SkillTreeSystem::getInstance();
                long long baseGold = 10;
                long long reward = skill.adjustGoldRewardForCombat(baseGold);
                ws.gold += reward;
                skill.addXp(Game::SkillTreeType::Combat, skill.xpForCombatKill(baseGold));
            }
        auto drops = m.getDrops();
        if (_map) {
            int c = 0;
            int r = 0;
            _map->worldToTileIndex(m.pos, c, r);
            for (auto t : drops) {
                _map->spawnDropAt(c, r, static_cast<int>(t), 1);
            }
        }
            if (m.sprite && m.sprite->getParent()) {
                m.sprite->removeFromParent();
            }
            m.sprite = nullptr;
            _monsters.erase(_monsters.begin() + static_cast<long>(i));
            continue;
        }
        ++i;
    }
    refreshVisuals();
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
                float scale = 1.0f;
                if (m.type == Game::Monster::Type::Bug || m.type == Game::Monster::Type::Ghost) {
                    scale = 0.3f;
                }
                spr->setScale(scale);
                spr->setAnchorPoint(Vec2(0.5f, 0.0f));
                spr->setPosition(m.pos);
                _worldNode->addChild(spr, 3);
                m.sprite = spr;
            }
        } else if (m.sprite) {
            m.sprite->setPosition(m.pos);
        }
    }
}

} // namespace Controllers
