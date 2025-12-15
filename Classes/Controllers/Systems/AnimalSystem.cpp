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
            case Game::AnimalType::Chicken: return Game::ItemType::Egg;
            case Game::AnimalType::Cow: return Game::ItemType::Milk;
            case Game::AnimalType::Sheep: return Game::ItemType::Wool;
        }
        return Game::ItemType::Egg;
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

// 构造：从全局 WorldState 还原农场动物实例，并附着到当前地图
AnimalSystem::AnimalSystem(Controllers::IMapController* map, cocos2d::Node* worldNode)
    : _map(map), _worldNode(worldNode) {
    auto& ws = Game::globalState();
    for (const auto& a : ws.farmAnimals) {
        Instance inst;
        inst.animal = a;
        inst.velocity = Vec2::ZERO;
        inst.idleTimer = randomFloat(0.5f, 2.0f);
        _animals.push_back(inst);
    }
}

// 确保某个动物实例拥有精灵与成长/喂食状态标签
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
    if (inst.sprite && !inst.growthLabel) {
        auto label = Label::createWithTTF("", "fonts/arial.ttf", 24);
        if (!label) return;
        label->setColor(Color3B::BLACK);
        label->setAnchorPoint(Vec2(0.5f, 0.0f));
        float spriteScale = inst.sprite->getScale();
        if (spriteScale > 0.0f) {
            label->setScale(1.0f / spriteScale);
        }
        inst.sprite->addChild(label, 1);
        inst.growthLabel = label;
    }
}

// 刷新头顶成长/喂食状态文本（Adult / 剩余天数 + Full/Hungry）
void AnimalSystem::updateGrowthLabel(Instance& inst) {
    if (!inst.growthLabel || !inst.sprite) return;
    int need = matureDays(inst.animal.type);
    std::string text;
    if (inst.animal.isAdult) {
        text = "Adult";
    } else {
        int left = need - inst.animal.ageDays;
        if (left < 0) left = 0;
        text = StringUtils::format("%dd", left);
    }
    if (inst.animal.fedToday) {
        text += " Full";
    } else {
        text += " Hungry";
    }
    inst.growthLabel->setString(text);
    auto cs = inst.sprite->getContentSize();
    float h = cs.height * inst.sprite->getScaleY();
    Vec2 pos(8.0f, h + 12.0f);
    inst.growthLabel->setPosition(pos);
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
    inst.idleTimer = randomFloat(0.5f, 2.0f);
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
        if (inst.idleTimer > 0.0f) {
            inst.idleTimer -= dt;
            if (inst.idleTimer < 0.0f) inst.idleTimer = 0.0f;
            if (inst.sprite) {
                inst.sprite->setPosition(inst.animal.pos);
                _map->sortActorWithEnvironment(inst.sprite);
            }
            updateGrowthLabel(inst);
            continue;
        }
        Vec2 pos = inst.animal.pos;
        Vec2 toTarget = inst.animal.target - pos;
        float dist = toTarget.length();
        if (dist < s * 0.1f) {
            float chooseIdle = randomFloat(0.0f, 1.0f);
            if (chooseIdle < 0.5f) {
                inst.idleTimer = randomFloat(1.0f, 3.0f);
                if (inst.sprite) {
                    inst.sprite->setPosition(inst.animal.pos);
                    _map->sortActorWithEnvironment(inst.sprite);
                }
                updateGrowthLabel(inst);
                continue;
            }
            float angle = randomFloat(0.0f, 6.2831853f);
            float radius = randomFloat(s * 0.5f, inst.animal.wanderRadius * s);
            Vec2 offset(std::cos(angle) * radius, std::sin(angle) * radius);
            Vec2 candidate = pos + offset;
            Vec2 clamped = _map->clampPosition(pos, candidate, s * 0.6f);
            Vec2 foot = clamped + Vec2(0, -s * 0.5f);
            if (_map->collides(foot, s * 0.5f)) {
                inst.idleTimer = randomFloat(0.8f, 2.0f);
                if (inst.sprite) {
                    inst.sprite->setPosition(inst.animal.pos);
                    _map->sortActorWithEnvironment(inst.sprite);
                }
                updateGrowthLabel(inst);
                continue;
            }
            inst.animal.target = clamped;
            toTarget = inst.animal.target - pos;
            dist = toTarget.length();
        }
        if (dist > 1e-3f) {
            Vec2 dir = toTarget / dist;
            float speed = inst.animal.speed * 0.4f;
            Vec2 next = pos + dir * speed * dt;
            Vec2 clamped = _map->clampPosition(pos, next, s * 0.6f);
            Vec2 foot = clamped + Vec2(0, -s * 0.5f);
            if (_map->collides(foot, s * 0.5f)) {
                inst.animal.pos = pos;
                inst.idleTimer = randomFloat(0.8f, 2.0f);
            } else {
                inst.animal.pos = clamped;
            }
        }
        if (inst.sprite) {
            inst.sprite->setPosition(inst.animal.pos);
            _map->sortActorWithEnvironment(inst.sprite);
        }
        updateGrowthLabel(inst);
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

void advanceAnimalsDailyWorldOnly() {
    auto& ws = Game::globalState();
    for (auto& a : ws.farmAnimals) {
        if (a.fedToday) {
            a.ageDays += 1;
            if (!a.isAdult && a.ageDays >= matureDays(a.type)) {
                a.isAdult = true;
            }
        }
        if (a.isAdult && a.fedToday) {
            Game::ItemType prod = productFor(a.type);
            int qty = productQty(a.type);
            if (qty > 0) {
                Game::Drop d{ prod, a.pos, qty };
                ws.farmDrops.push_back(d);
            }
        }
        a.fedToday = false;
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
