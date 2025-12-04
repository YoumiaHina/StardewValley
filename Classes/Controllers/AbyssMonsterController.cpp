#include "Controllers/AbyssMonsterController.h"
#include "cocos2d.h"
#include <algorithm>

using namespace cocos2d;

namespace Controllers {

void AbyssMonsterController::generateInitialWave() {
    _monsters.clear();
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> countDist(3,5);
    int count = countDist(rng);
    std::uniform_real_distribution<float> distX(0.0f, _map->getContentSize().width);
    std::uniform_real_distribution<float> distY(0.0f, _map->getContentSize().height);
    for (int i = 0; i < count; ++i) {
        Monster m = makeMonsterForTheme(_map->currentTheme());
        Vec2 p(distX(rng), distY(rng));
        m.pos = p;
        _monsters.push_back(m);
    }
    refreshVisuals();
}

void AbyssMonsterController::update(float dt) {
    _respawnAccum += dt;
    if (_respawnAccum >= 30.0f) {
        _respawnAccum = 0.0f;
        if (_monsters.size() < 8) {
            _monsters.push_back(makeMonsterForTheme(_map->currentTheme()));
            refreshVisuals();
        }
    }
}

void AbyssMonsterController::resetFloor() {
    _monsters.clear();
    _respawnAccum = 0.0f;
}

void AbyssMonsterController::applyDamageAt(const Vec2& worldPos, int baseDamage) {
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
            // 掉落：简化为添加对应主题碎片 1 个
            auto theme = _map->currentTheme();
            Game::ItemType drop = Game::ItemType::Fiber; // placeholder
            if (theme == AbyssMapController::Theme::Ice) drop = Game::ItemType::ParsnipSeed; // placeholder swap
            if (auto inv = Game::globalState().inventory) {
                inv->addItems(drop, 1);
            }
            _monsters.erase(_monsters.begin() + idx);
            refreshVisuals();
        }
    }
}

AbyssMonsterController::Monster AbyssMonsterController::makeMonsterForTheme(AbyssMapController::Theme theme) {
    Monster m{};
    switch (theme) {
        case AbyssMapController::Theme::Rock:
            m.type = Monster::Type::RockSlime; m.hp=80; m.maxHp=80; m.dmg=8; m.def=3; m.searchRangeTiles=4; break;
        case AbyssMapController::Theme::Ice:
            m.type = Monster::Type::IceBat; m.hp=60; m.maxHp=60; m.dmg=12; m.def=1; m.searchRangeTiles=6; break;
        case AbyssMapController::Theme::Lava:
            m.type = Monster::Type::LavaCrab; m.hp=200; m.maxHp=200; m.dmg=20; m.def=8; m.searchRangeTiles=3; break;
    }
    m.elite = false;
    return m;
}

void AbyssMonsterController::refreshVisuals() {
    if (!_monsterDraw) {
        _monsterDraw = DrawNode::create();
        if (_worldNode) _worldNode->addChild(_monsterDraw, 3);
    }
    _monsterDraw->clear();
    for (const auto& m : _monsters) {
        Color4F bodyColor = Color4F(0.2f,0.75f,0.25f,1.0f);
        switch (m.type) {
            case Monster::Type::RockSlime: bodyColor = Color4F(0.2f,0.75f,0.25f,1.0f); break;
            case Monster::Type::BurrowBug: bodyColor = Color4F(0.45f,0.35f,0.25f,1.0f); break;
            case Monster::Type::IceBat:    bodyColor = Color4F(0.35f,0.55f,0.85f,1.0f); break;
            case Monster::Type::IceMage:   bodyColor = Color4F(0.50f,0.70f,0.95f,1.0f); break;
            case Monster::Type::LavaCrab:  bodyColor = Color4F(0.85f,0.35f,0.20f,1.0f); break;
            case Monster::Type::LavaWarlock: bodyColor = Color4F(0.80f,0.20f,0.25f,1.0f); break;
            case Monster::Type::BossGuardian: bodyColor = Color4F(0.90f,0.15f,0.15f,1.0f); break;
        }
        float s = static_cast<float>(GameConfig::TILE_SIZE);
        _monsterDraw->drawSolidCircle(m.pos, s*0.35f, 0.0f, 18, bodyColor);
        // HP bar
        float pct = (m.maxHp > 0) ? (static_cast<float>(m.hp) / static_cast<float>(m.maxHp)) : 0.0f;
        pct = std::max(0.0f, std::min(1.0f, pct));
        Vec2 barOrigin = m.pos + Vec2(-s*0.35f, s*0.45f);
        Vec2 barEnd    = m.pos + Vec2(s*0.35f,  s*0.45f);
        _monsterDraw->drawLine(barOrigin, barEnd, Color4F(0,0,0,0.5f));
        Vec2 barFillEnd = barOrigin + (barEnd - barOrigin) * pct;
        _monsterDraw->drawLine(barOrigin, barFillEnd, Color4F(0.1f,0.9f,0.1f,1.0f));
    }
}

} // namespace Controllers