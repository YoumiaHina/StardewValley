#include "Controllers/Systems/AnimalSystem.h"
#include "Game/Animals/AnimalBase.h"
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
}

AnimalSystem::AnimalSystem(Controllers::IMapController* map, cocos2d::Node* worldNode)
    : _map(map), _worldNode(worldNode) {
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
    inst.velocity = Vec2::ZERO;
    _animals.push_back(inst);
    ensureSprite(_animals.back());
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
}

}

