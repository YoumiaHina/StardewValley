#include "Controllers/Systems/AnimalSystem.h"
#include "Game/Animals/AnimalBase.h"
#include "Game/WorldState.h"
#include "Game/Item.h"
#include "Game/SkillTree/SkillTreeSystem.h"
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
    // 产物掉落描述：
    // - 用于把“动物当日产出”暂存为一条掉落信息（类型/数量/世界坐标）。
    // - 由每日推进逻辑生成，随后交由 IMapController::spawnDropAt 落地到地图掉落系统。
    struct ProducedDrop {
        Game::ItemType type = Game::ItemType::Egg;
        int qty = 0;
        cocos2d::Vec2 pos;
    };

    // 获取某类动物从幼年到成年的所需“有效喂食天数”。
    int matureDays(Game::AnimalType type);
    // 获取某类动物成年且当日被喂食时产出的物品类型。
    Game::ItemType productFor(Game::AnimalType type);
    // 获取某类动物当日产出数量（可能包含随机）。
    int productQty(Game::AnimalType type);

    // 推进单只动物的“离线一天”：
    // - 若 fedToday=true：年龄 +1，并在达到成熟天数后标记为成年。
    // - 若成年且 fedToday=true：生成当日产物（通过 producedDrop 输出）。
    // - 结尾统一清空 fedToday（跨日重置）。
    // 返回值：本日是否产生了可掉落的产物（qty>0）。
    bool advanceAnimalOneDay(Game::Animal& animal, ProducedDrop* producedDrop) {
        if (producedDrop) {
            producedDrop->qty = 0;
            producedDrop->pos = animal.pos;
        }

        if (animal.fedToday) {
            animal.ageDays += 1;
            if (!animal.isAdult && animal.ageDays >= matureDays(animal.type)) {
                animal.isAdult = true;
            }
        }

        bool produced = false;
        if (animal.isAdult && animal.fedToday) {
            Game::ItemType prod = productFor(animal.type);
            int qty = productQty(animal.type);
            {
                auto& skill = Game::SkillTreeSystem::getInstance();
                qty = skill.adjustAnimalProductQuantityForHusbandry(prod, qty);
                skill.addXp(Game::SkillTreeType::AnimalHusbandry, skill.xpForAnimalProduct(prod, qty));
            }
            if (producedDrop) {
                producedDrop->type = prod;
                producedDrop->qty = qty;
                producedDrop->pos = animal.pos;
            }
            produced = (qty > 0);
        }

        animal.fedToday = false;
        return produced;
    }

    // 根据动物类型获取其静态行为数据（速度/半径/贴图路径）。
    const Game::AnimalBase& behaviorFor(Game::AnimalType type) {
        switch (type) {
            case Game::AnimalType::Chicken: return Game::chickenAnimalBehavior();
            case Game::AnimalType::Cow: return Game::cowAnimalBehavior();
            case Game::AnimalType::Sheep: return Game::sheepAnimalBehavior();
        }
        return Game::chickenAnimalBehavior();
    }

    // 全局随机数引擎：用于动物游走与产物数量随机。
    std::mt19937& rng() {
        static std::mt19937 eng{ std::random_device{}() };
        return eng;
    }

    // 生成 [a,b] 区间内的随机浮点数。
    float randomFloat(float a, float b) {
        std::uniform_real_distribution<float> dist(a, b);
        return dist(rng());
    }

    // 生成 [a,b] 区间内的随机整数。
    int randomInt(int a, int b) {
        std::uniform_int_distribution<int> dist(a, b);
        return dist(rng());
    }

    // 饲料可接受性判定：
    // - 小鸡：接受任意种子（复用 Game::isSeed）。
    // - 牛/羊：仅接受萝卜（示例规则，可后续替换为更完整的饲料系统）。
    bool acceptsFeed(Game::AnimalType type, Game::ItemType feed) {
        if (type == Game::AnimalType::Chicken) {
            return Game::isSeed(feed);
        }
        if (type == Game::AnimalType::Cow || type == Game::AnimalType::Sheep) {
            return feed == Game::ItemType::Parsnip;
        }
        return false;
    }

    // 动物产物映射：将动物类型映射为当日产物物品类型。
    Game::ItemType productFor(Game::AnimalType type) {
        switch (type) {
            case Game::AnimalType::Chicken: return Game::ItemType::Egg;
            case Game::AnimalType::Cow: return Game::ItemType::Milk;
            case Game::AnimalType::Sheep: return Game::ItemType::Wool;
        }
        return Game::ItemType::Egg;
    }

    // 动物产物数量规则：将动物类型映射为当日产物数量（部分带随机）。
    int productQty(Game::AnimalType type) {
        switch (type) {
            case Game::AnimalType::Chicken: return randomInt(1, 3);
            case Game::AnimalType::Cow: return 2;
            case Game::AnimalType::Sheep: return randomInt(1, 2);
        }
        return 1;
    }

    // 成熟天数规则：将动物类型映射为“喂食累计天数达到后成年”。
    int matureDays(Game::AnimalType type) {
        switch (type) {
            case Game::AnimalType::Chicken: return 3;
            case Game::AnimalType::Cow: return 5;
            case Game::AnimalType::Sheep: return 5;
        }
        return 3;
    }
}

// 构造：从全局 WorldState 还原农场动物实例到运行时缓存。
// - 仅恢复运行时状态（位置/目标/成长/喂食等），精灵节点延迟到 ensureSprite 创建。
// - map/worldNode 用于后续挂载精灵与碰撞/掉落交互。
AnimalSystem::AnimalSystem(Controllers::IMapController* map, cocos2d::Node* worldNode)
    : _map(map), _worldNode(worldNode) {
    auto& ws = Game::globalState();
    for (const auto& a : ws.farmAnimals) {
        Instance inst;
        inst.animal = a;
        inst.idleTimer = randomFloat(0.5f, 2.0f);
        _animals.push_back(inst);
    }
}

// 确保实例拥有精灵与状态标签：
// - 首次创建：按行为表选择贴图，按 tileSize 做高度缩放，并挂载到地图 actor 层。
// - 标签：挂在精灵子节点上，用于显示成年/剩余天数与饥饿状态。
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
        auto label = Label::createWithTTF("", "fonts/arial.ttf", 12);
        if (!label) return;
        label->setColor(Color3B::BLACK);
        label->setAnchorPoint(Vec2(0.5f, 0.0f));
        float spriteScale = inst.sprite->getScale();
        if (spriteScale > 0.0f) {
            label->setScale(0.6f / spriteScale);
        }
        inst.sprite->addChild(label, 1);
        inst.growthLabel = label;
    }
}

// 刷新头顶状态文本：
// - 未成年：显示距离成年还差的“有效喂食天数”。
// - 已成年：显示 Adult。
// - 追加当日喂食状态（Full/Hungry）。
// - 位置：基于精灵的实际缩放后高度，在头顶上方偏移一定像素以避免遮挡。
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
    Vec2 pos(8.0f, h + 28.0f);
    inst.growthLabel->setPosition(pos);
}

// 生成一只新动物到系统：
// - 初始化运行时数据（位置/目标/速度/游走半径/成长/喂食）。
// - 立即创建并挂载精灵（如 map/worldNode 有效）。
// - 写回 WorldState（用于存档/跨场景/离线推进）。
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
    inst.idleTimer = randomFloat(0.5f, 2.0f);
    _animals.push_back(inst);
    ensureSprite(_animals.back());
    syncSave();
}

// 购买动物：
// - price 由上层决定（通常使用 Game::animalPrice），此处只负责校验余额并扣费。
// - 成功时会调用 spawnAnimal 并写回 WorldState。
bool AnimalSystem::buyAnimal(Game::AnimalType type, const cocos2d::Vec2& pos, long long price) {
    if (!_map || !_worldNode) return false;
    if (price < 0) price = 0;
    auto& ws = Game::globalState();
    if (ws.gold < price) return false;
    spawnAnimal(type, pos);
    ws.gold -= price;
    return true;
}

// 每帧更新：
// - 维护每只动物的游走/停留状态，做简单的“随机目标点 + 碰撞回退”。
// - 同步精灵位置与深度排序（与环境遮挡关系）。
// - 每帧把动物列表回写到 WorldState，确保存档与系统外读取的一致性。
void AnimalSystem::update(float dt) {
    if (!_map) return;
    auto& ws = Game::globalState();
    if (ws.lastScene != static_cast<int>(Game::SceneKind::Farm)) return;
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
    syncSave();
}

void advanceAnimalsDaily(Controllers::IMapController* map) {
    auto& ws = Game::globalState();
    Controllers::IMapController* dropMap = (map && map->isFarm()) ? map : nullptr;
    float tileSize = dropMap ? dropMap->tileSize() : 0.0f;
    for (auto& a : ws.farmAnimals) {
        ProducedDrop drop;
        bool produced = advanceAnimalOneDay(a, &drop);
        if (!produced) continue;

        auto appendWorldDrop = [&ws, &drop]() {
            ws.farmDrops.push_back(Game::Drop{ drop.type, drop.pos, drop.qty });
        };

        if (!dropMap || tileSize <= 0.0f) {
            appendWorldDrop();
            continue;
        }

        int c = 0;
        int r = 0;
        cocos2d::Vec2 clamped = dropMap->clampPosition(drop.pos, drop.pos, tileSize * 0.1f);
        dropMap->worldToTileIndex(clamped, c, r);
        if (dropMap->inBounds(c, r)) dropMap->spawnDropAt(c, r, static_cast<int>(drop.type), drop.qty);
        else appendWorldDrop();
    }
}

// 尝试喂食：
// - 从玩家附近选择最近的一只“未喂食且接受该饲料”的动物。
// - 仅在系统内部标记 fedToday 并输出 consumedQty；背包扣除由调用方完成。
// - 成功后写回 WorldState，确保当日状态持久化。
bool AnimalSystem::tryFeedAnimal(const cocos2d::Vec2& playerPos, Game::ItemType feedType, int& consumedQty) {
    consumedQty = 0;
    if (!_map) return false;
    float s = _map->tileSize();
    float maxDist = s * 1.2f;
    Instance* best = nullptr;
    float bestDist = 1e9f;
    for (auto& inst : _animals) {
        if (inst.animal.fedToday) continue;
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
    {
        auto& skill = Game::SkillTreeSystem::getInstance();
        skill.addXp(Game::SkillTreeType::AnimalHusbandry, 6);
    }
    syncSave();
    return true;
}

void AnimalSystem::syncSave() {
    auto& ws = Game::globalState();
    ws.farmAnimals.clear();
    ws.farmAnimals.reserve(_animals.size());
    for (const auto& it : _animals) {
        ws.farmAnimals.push_back(it.animal);
    }
}

}
