#include "Controllers/Systems/AnimalSystem.h"
#include "Game/Animals/AnimalBase.h"
#include "Game/WorldState.h"
#include "Game/Crop.h"
#include "Game/Item.h"
#include <random>
#include <cmath>

using namespace cocos2d;

namespace Game {
    const AnimalBase& chickenAnimalBehavior();
    const AnimalBase& cowAnimalBehavior();
    const AnimalBase& sheepAnimalBehavior();
}

namespace Controllers {

namespace {
    const Game::AnimalBase& behaviorFor(Game::AnimalType type) {
        switch (type) {
            case Game::AnimalType::Chicken: return Game::chickenAnimalBehavior();
            case Game::AnimalType::Cow: return Game::cowAnimalBehavior();
            case Game::AnimalType::Sheep: return Game::sheepAnimalBehavior();
        }
        return Game::chickenAnimalBehavior();
    }

    std::mt19937& rng() {
        static std::mt19937 eng{ std::random_device{}() };
        return eng;
    }

    float randomFloat(float a, float b) {
        std::uniform_real_distribution<float> dist(a, b);
        return dist(rng());
    }

    int randomInt(int a, int b) {
        std::uniform_int_distribution<int> dist(a, b);
        return dist(rng());
    }

    bool acceptsFeed(Game::AnimalType type, Game::ItemType feed) {
        if (type == Game::AnimalType::Chicken) {
            return Game::isSeed(feed);
        }
        if (type == Game::AnimalType::Cow || type == Game::AnimalType::Sheep) {
            return feed == Game::ItemType::Parsnip;
        }
        return false;
    }

    Game::ItemType productFor(Game::AnimalType type) {
        switch (type) {
            case Game::AnimalType::Chicken: return static_cast<Game::ItemType>(200);
            case Game::AnimalType::Cow: return static_cast<Game::ItemType>(201);
            case Game::AnimalType::Sheep: return static_cast<Game::ItemType>(202);
        }
        return static_cast<Game::ItemType>(200);
    }

    int productQty(Game::AnimalType type) {
        switch (type) {
            case Game::AnimalType::Chicken: return randomInt(1, 3);
            case Game::AnimalType::Cow: return 2;
            case Game::AnimalType::Sheep: return randomInt(1, 2);
        }
        return 1;
    }

    int matureDays(Game::AnimalType type) {
        switch (type) {
            case Game::AnimalType::Chicken: return 3;
            case Game::AnimalType::Cow: return 5;
            case Game::AnimalType::Sheep: return 5;
        }
        return 3;
    }
}

AnimalSystem::AnimalSystem(Controllers::IMapController* map, cocos2d::Node* worldNode)
    : _map(map), _worldNode(worldNode) {
    auto& ws = Game::globalState();
    for (const auto& a : ws.farmAnimals) {
        Instance inst;
        inst.animal = a;
        inst.velocity = Vec2::ZERO;
        _animals.push_back(inst);
    }
}

void AnimalSystem::ensureSprite(Instance& inst) {
    if (!inst.sprite && _worldNode && _map) {
        const auto& beh = behaviorFor(inst.animal.type);
        auto sp = Sprite::create(beh.texturePath());
        if (!sp) return;
        sp->setAnchorPoint(Vec2(0.5f, 0.0f));
        float s = _map->tileSize();
        auto cs = sp->getContentSize();
        if (cs.width > 0 && cs.height > 0) {
            float targetH = s;
            float scale = targetH / cs.height;
            sp->setScale(scale);
        }
        sp->setPosition(inst.animal.pos);
        _map->addActorToMap(sp, 20);
        _map->sortActorWithEnvironment(sp);
        inst.sprite = sp;
    }
}

void AnimalSystem::spawnAnimal(Game::AnimalType type, const cocos2d::Vec2& pos) {
    if (!_map || !_worldNode) return;
    Instance inst;
    inst.animal.type = type;
    inst.animal.pos = pos;
    inst.animal.target = pos;
    const auto& beh = behaviorFor(type);
    inst.animal.speed = beh.moveSpeed();
    inst.animal.wanderRadius = beh.wanderRadius();
    inst.animal.ageDays = 0;
    inst.animal.isAdult = false;
    inst.animal.fedToday = false;
    inst.velocity = Vec2::ZERO;
    _animals.push_back(inst);
    ensureSprite(_animals.back());
    auto& ws = Game::globalState();
    ws.farmAnimals.clear();
    for (const auto& it : _animals) {
        ws.farmAnimals.push_back(it.animal);
    }
}

void AnimalSystem::update(float dt) {
    if (!_map) return;
    float s = _map->tileSize();
    for (auto& inst : _animals) {
        ensureSprite(inst);
        Vec2 pos = inst.animal.pos;
        Vec2 toTarget = inst.animal.target - pos;
        float dist = toTarget.length();
        if (dist < s * 0.1f) {
            float angle = randomFloat(0.0f, 6.2831853f);
            float radius = randomFloat(s * 0.5f, inst.animal.wanderRadius * s);
            Vec2 offset(std::cos(angle) * radius, std::sin(angle) * radius);
            Vec2 candidate = pos + offset;
            Vec2 clamped = _map->clampPosition(pos, candidate, s * 0.4f);
            inst.animal.target = clamped;
            toTarget = inst.animal.target - pos;
            dist = toTarget.length();
        }
        if (dist > 1e-3f) {
            Vec2 dir = toTarget / dist;
            Vec2 next = pos + dir * inst.animal.speed * dt;
            Vec2 clamped = _map->clampPosition(pos, next, s * 0.4f);
            inst.animal.pos = clamped;
        }
        if (inst.sprite) {
            inst.sprite->setPosition(inst.animal.pos);
            _map->sortActorWithEnvironment(inst.sprite);
        }
    }
    auto& ws = Game::globalState();
    ws.farmAnimals.clear();
    for (const auto& it : _animals) {
        ws.farmAnimals.push_back(it.animal);
    }
}

void AnimalSystem::advanceAnimalsDaily() {
    if (!_map) return;
    auto& ws = Game::globalState();
    for (auto& inst : _animals) {
        if (inst.animal.fedToday) {
            inst.animal.ageDays += 1;
            if (!inst.animal.isAdult && inst.animal.ageDays >= matureDays(inst.animal.type)) {
                inst.animal.isAdult = true;
            }
        }
        if (inst.animal.isAdult && inst.animal.fedToday) {
            Game::ItemType prod = productFor(inst.animal.type);
            int qty = productQty(inst.animal.type);
            int c = 0;
            int r = 0;
            _map->worldToTileIndex(inst.animal.pos, c, r);
            if (_map->inBounds(c, r) && qty > 0) {
                _map->spawnDropAt(c, r, static_cast<int>(prod), qty);
            }
        }
        inst.animal.fedToday = false;
    }
    ws.farmAnimals.clear();
    for (const auto& it : _animals) {
        ws.farmAnimals.push_back(it.animal);
    }
    if (_map) {
        _map->refreshDropsVisuals();
    }
}

bool AnimalSystem::tryFeedAnimal(const cocos2d::Vec2& playerPos, Game::ItemType feedType, int& consumedQty) {
    consumedQty = 0;
    if (!_map) return false;
    float s = _map->tileSize();
    float maxDist = s * 1.2f;
    Instance* best = nullptr;
    float bestDist = 1e9f;
    for (auto& inst : _animals) {
        if (!acceptsFeed(inst.animal.type, feedType)) continue;
        float d = playerPos.distance(inst.animal.pos);
        if (d <= maxDist && d < bestDist) {
            bestDist = d;
            best = &inst;
        }
    }
    if (!best) return false;
    best->animal.fedToday = true;
    consumedQty = 1;
    auto& ws = Game::globalState();
    ws.farmAnimals.clear();
    for (const auto& it : _animals) {
        ws.farmAnimals.push_back(it.animal);
    }
    return true;
}

}
