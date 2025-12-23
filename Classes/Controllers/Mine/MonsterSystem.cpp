#include "Controllers/Mine/MonsterSystem.h"
#include "cocos2d.h"
#include "Game/SkillTree/SkillTreeSystem.h"
#include "Game/Monster/MonsterBase.h"
#include <algorithm>
#include <string>

using namespace cocos2d;

namespace Controllers {

namespace {
    // rng：本控制器内部使用的随机数引擎，用于刷怪时进行类型抽样。
    // - 使用 std::mt19937 表示一个梅森旋转算法的伪随机数引擎，比 C 标准库
    //   的 rand() 质量更高；
    // - 声明为 static 局部变量，保证整个进程生命周期内只构造一次，可视作
    //   “懒汉单例”：第一次调用 rng() 时创建，之后重复复用。
    std::mt19937& rng() {
        static std::mt19937 eng{ std::random_device{}() };
        return eng;
    }

    // randomMonsterTypeForFloor：
    // 根据当前楼层返回一个随机怪物类型。不同楼层通过插值调整各类怪物的出现
    // 概率，实现：
    // - 低层：史莱姆为主，幽灵极少；
    // - 高层：逐渐提高幽灵比例，调整 bug 与各色史莱姆的占比，
    // 从而在不修改 TMX 的前提下实现“随楼层递进”的难度曲线。
    Game::MonsterType randomMonsterTypeForFloor(int floor) {
        int f = floor; // 工作变量：后续会对其进行“夹紧”处理
        if (f < 1) f = 1;
        if (f > 50) f = 50;
        float t = 0.0f; // 归一化后的 [0,1] 比例，表示楼层深度
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
        // uniform_real_distribution 负责把 rng() 输出的整数转换为 [0,1] 的浮点数，
        // 方便按“概率区间”做判断。
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float r = dist(rng());
        if (r < greenShare) return Game::MonsterType::GreenSlime;
        if (r < greenShare + blueShare) return Game::MonsterType::BlueSlime;
        if (r < slime) return Game::MonsterType::RedSlime;
        if (r < slime + bug) return Game::MonsterType::Bug;
        return Game::MonsterType::Ghost;
    }
}

// ~MineMonsterController：
// 控制器析构时，负责清理自己创建的所有 Cocos 节点，防止场景销毁后留下悬挂
// 子节点或重复 remove 的问题。
// - 对每一只怪物：如 sprite 仍在场景树上，则从父节点移除；
// - 对调试绘制节点 _monsterDraw：如仍挂在父节点上，同样移除。
// 可以类比 C 中“谁申请，谁释放”的习惯：这里所有 sprite/_monsterDraw 均由本类
// 创建，因此也在析构时统一释放。
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

// generateInitialWave：
// 生成当前楼层的初始怪物波次，一般在：
// - 场景进入矿洞楼层时；
// - 玩家通过楼梯/电梯下楼之后，
// 由外部先调用 resetFloor 清掉上一层状态，再调用本函数。
//
// 规则：
// - 若当前层为入口层（floor <= 0）：不刷怪，只刷新一次可视，确保调试绘制正确；
// - 若 TMX 中未配置 MonsterArea（monsterSpawnPoints 为空）：同样不刷怪；
// - 否则：从 TMX 提供的出生点列表中遍历，为每个点随机抽取一个怪物类型，
//   通过 Game::monsterInfoFor(type) 拿到其属性，设置初始血量与位置。
// 最后统一调用 refreshVisuals，把逻辑状态映射到精灵/调试节点。
void MineMonsterController::generateInitialWave() {
    _monsters.clear();
    if (_map && _map->currentFloor() <= 0) { refreshVisuals(); return; }
    auto spawns = _map->monsterSpawnPoints();
    if (spawns.empty()) {
        // 没有 MonsterArea：不刷怪
        refreshVisuals();
        return;
    }
    // 使用当前楼层决定怪物类型；若没有 map，则退化到楼层 1。
    int floor = _map ? _map->currentFloor() : 1;
    for (const auto& pt : spawns) {
        Game::MonsterType type = randomMonsterTypeForFloor(floor);
        // 这里局部构造 Monster，再 push_back 到 std::vector 中；
        // 等价于 C 里“先填一个结构体，再追加到动态数组尾部”。
        Monster m;
        m.type = type;
        const auto& info = Game::monsterInfoFor(type);
        m.hp = info.def_.hp;
        m.pos = pt;
        _monsters.push_back(m);
    }
    refreshVisuals();
}

// update：
// 每帧驱动怪物逻辑，包括：
// 1) 入口层与无 MonsterArea 的早退；
// 2) 简单的重生逻辑：每隔固定时间在数量不足时补刷一只怪；
// 3) 追踪玩家的移动与地图/玩家碰撞；
// 4) 怪物之间的成对分离，避免重叠在一起；
// 5) 将怪物碰撞体同步给 MineMapController，供玩家移动/工具判定参考；
// 6) 结算怪物对玩家的接触伤害，并在玩家死亡时清空所有怪物；
// 7) 调用 refreshVisuals，将最新位置与移动状态同步到精灵和动画。
void MineMonsterController::update(float dt) {
    if (_map && _map->currentFloor() <= 0) return; // 入口层：不刷新/重生
    // 没有 MonsterArea：不进行任何刷新/重生
    if (_map && _map->monsterSpawnPoints().empty()) return;
    // 使用一个累计计时器实现“每 30 秒刷一只怪”的简单重生逻辑。
    _respawnAccum += dt;
    if (_respawnAccum >= 30.0f) {
        _respawnAccum = 0.0f;
        if (_monsters.size() < 8) {
            int floor = _map ? _map->currentFloor() : 1;
            Game::MonsterType type = randomMonsterTypeForFloor(floor);
            Monster m;
            m.type = type;
            const auto& info = Game::monsterInfoFor(type);
            m.hp = info.def_.hp;
            _monsters.push_back(m);
        }
    }
    // TILE_SIZE 定义每一个瓦片的边长像素数，转换为 float 方便参与向量运算。
    float tileSize = static_cast<float>(GameConfig::TILE_SIZE);
    Vec2 playerPos = _getPlayerPos ? _getPlayerPos() : Vec2::ZERO;
    float monsterRadius = tileSize * 0.4f;
    float playerRadius = tileSize * 0.4f;

    for (auto& m : _monsters) {
        if (m.attackCooldown > 0.0f) {
            // std::max(a,b) 返回较大的那个，这里保证冷却时间不会减到负数。
            m.attackCooldown = std::max(0.0f, m.attackCooldown - dt);
        }
    }

    // 怪物朝玩家移动，带基础碰撞：不能穿过墙体/矿石/玩家
    for (auto& m : _monsters) {
        if (_getPlayerPos) {
            const auto& info = Game::monsterInfoFor(m.type);
            const auto& def = info.def_;
            float range = def.searchRangeTiles * tileSize;
            Vec2 delta = playerPos - m.pos;
            // Vec2::length() 返回向量长度，即两点之间的直线距离。
            float dist = delta.length();
            if (dist > 0.001f && dist <= range) {
                // 单位化方向向量：除以长度，得到长度为 1 的方向，用于位移计算。
                Vec2 dir = delta / dist;
                Vec2 proposed = m.pos + dir * def.moveSpeed * dt;
                bool blocked = false; // 记录是否被墙体/矿石/玩家阻挡
                if (_map && def.isCollisionAffected) {
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
                    m.velocity = dir * def.moveSpeed;
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
            float dist = delta.length(); // 两只怪物之间的距离
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
            // Rect(x,y,w,h) 用左下角坐标和宽高构造矩形；这里使怪物居中在矩形内。
            Rect rc(m.pos.x - half, m.pos.y - half, ts, ts);
            colliders.push_back(rc);
        }
        _map->setMonsterColliders(colliders);
    }
    auto& ws = Game::globalState();
    for (auto& m : _monsters) {
        float dist = m.pos.distance(playerPos);
        float attackRange = monsterRadius + playerRadius + 2.0f;
        const auto& info = Game::monsterInfoFor(m.type);
        const auto& def = info.def_;
        if (dist < attackRange && m.attackCooldown <= 0.0f && def.dmg > 0) {
            ws.hp = std::max(0, ws.hp - def.dmg);
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
            // clear() 仅清空调试图形内容，不会从场景树中移除节点本身。
            _monsterDraw->clear();
        }
        return;
    }
    refreshVisuals();
}

// resetFloor：
// 切换楼层时由外部调用，用于彻底清空上一层的怪物状态。
// - 移除所有怪物精灵节点并将指针置空；
// - 清空内部 _monsters 容器；
// - 将怪物碰撞体清空回写给 MineMapController；
// - 重置重生计时器，使新楼层从“无怪物”状态开始，待 generateInitialWave
//   重新生成。
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
}

// applyAreaDamage：
// 对一组瓦片坐标上的怪物批量结算伤害，用于范围技能/爆炸等：
// - 利用 MapController::worldToTileIndex 将怪物当前位置映射到整数瓦片坐标；
// - 若怪物所在瓦片在 tiles 列表内，则认为命中，按单体伤害规则扣血；
// - hp 降为 0 时，结算金币/经验、生成掉落并播放死亡动画，与 applyDamageAt
//   保持相同的死亡处理逻辑；
// - 采用 while 循环 + 手动递增索引，在删除元素时避免越界。
void MineMonsterController::applyAreaDamage(const std::vector<std::pair<int,int>>& tiles, int baseDamage) {
    if (!_map || tiles.empty()) return;
    // 使用 lambda 表达式封装“怪物是否位于攻击瓦片列表中”的判断逻辑：
    // - [this,&tiles] 捕获 this 指针与 tiles 引用，便于在内部访问成员函数和参数；
    // - 参数 m 是只读引用，避免复制 Monster 结构体。
    auto matches = [this,&tiles](const Monster& m) {
        int c = 0, r = 0;
        _map->worldToTileIndex(m.pos, c, r);
        for (const auto& t : tiles) {
            if (c == t.first && r == t.second) return true;
        }
        return false;
    };
    // 这里使用手动管理索引的 for 循环，而不是 range-for：
    // - 在循环过程中可能会 erase 元素，必须避免迭代器失效问题；
    // - 每次只在需要时才递增 i，下文在 erase 后不递增即可安全访问下一个元素。
    for (std::size_t i = 0; i < _monsters.size();) {
        // 拷贝一份 Monster 到局部变量 m，方便修改 hp 等字段；
        // 最后只把需要同步的字段写回 _monsters[i]。
        Monster m = _monsters[i];
        if (!matches(m)) { ++i; continue; }
        const auto& info = Game::monsterInfoFor(m.type);
        int def = info.def_.def;
        int dmg = std::max(0, baseDamage - def);
        m.hp -= dmg;
        if (m.hp <= 0) {
            auto& ws = Game::globalState();
            auto& skill = Game::SkillTreeSystem::getInstance();
            long long baseGold = 10; // 基础金币奖励
            long long reward = skill.adjustGoldRewardForCombat(baseGold);
            ws.gold += reward;
            skill.addXp(Game::SkillTreeType::Combat, skill.xpForCombatKill(baseGold));
            const auto& info = Game::monsterInfoFor(m.type);
            auto drops = info.drops_; // 使用 auto 让编译器推导具体容器类型
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
                info.playDeathAnimation(sprite, [sprite]() {
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

// refreshVisuals：
// 将逻辑层的 _monsters 状态同步到渲染层：
// - 懒创建 _monsterDraw：用于未来的调试可视化（碰撞体/路径等），挂在 worldNode
//   上，仅在首次需要时创建，后续重复复用并在每次刷新前 clear；
// - 对每一只怪物：
//   - 如还没有 sprite，则在 worldNode 下创建一个空 Sprite，并设置锚点与
//     初始位置；纹理与动画由 MonsterBase 的 playXXXAnimation 决定；
//   - 若已有 sprite，则仅更新位置；
//   - 根据 velocity 的长度判断是播放移动动画还是静止动画：
//     - 有速度：调用 monsterInfoFor(type).playMoveAnimation；
//     - 无速度：调用 playStaticAnimation。
// 控制器本身不关心具体贴图路径或帧序列，只负责把“什么时候动/停”这一信息
// 传递给行为层。
void MineMonsterController::refreshVisuals() {
    if (!_monsterDraw) {
        _monsterDraw = DrawNode::create();
        if (_worldNode) _worldNode->addChild(_monsterDraw, 4);
    }
    _monsterDraw->clear();
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    for (auto& m : _monsters) {
        if (!m.sprite && _worldNode) {
            auto spr = Sprite::create();
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
                info.playMoveAnimation(m.velocity, m.sprite);
            } else {
                info.playStaticAnimation(m.sprite);
            }
        }
    }
}

} // namespace Controllers
