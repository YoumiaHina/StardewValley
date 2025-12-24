#include "Controllers/Systems/FishingController.h"
#include "Game/GameConfig.h"
#include "Game/Item.h"
#include "Game/SkillTree/SkillTreeSystem.h"
#include "Game/WorldState.h"

using namespace cocos2d;

namespace {
struct FestivalFishDef {
    const char* texturePath;
    const char* displayName;
    Game::ItemType itemType;
};

// 钓鱼节可获得的特殊鱼定义表。
const FestivalFishDef kFestivalFish[] = {
    { "fish/carp.png", "Carp", Game::ItemType::Carp },
    { "fish/Bream.png", "Bream", Game::ItemType::BreamFish },
    { "fish/Sardine.png", "Sardine", Game::ItemType::Sardine },
    { "fish/Salmon.png", "Salmon", Game::ItemType::Salmon },
    { "fish/Rainbow_Trout.png", "Rainbow Trout", Game::ItemType::RainbowTrout },
    { "fish/Midnight_Carp.png", "Midnight Carp", Game::ItemType::MidnightCarp },
    { "fish/Largemouth_Bass.png", "Largemouth Bass", Game::ItemType::LargemouthBass },
    { "fish/Sturgeon.png", "Sturgeon", Game::ItemType::Sturgeon },
    { "fish/Smallmouth_Bass.png", "Smallmouth Bass", Game::ItemType::SmallmouthBass },
    { "fish/Tilapia.png", "Tilapia", Game::ItemType::Tilapia },
    { "fish/Tuna.png", "Tuna", Game::ItemType::Tuna },
    { "fish/globefish.png", "Globefish", Game::ItemType::Globefish },
    { "fish/Anchovy.png", "Anchovy", Game::ItemType::Anchovy },
    { "fish/Blue_Discus.png", "Blue Discus", Game::ItemType::BlueDiscus },
    { "fish/Clam.png", "Clam", Game::ItemType::Clam },
    { "fish/Crab.png", "Crab", Game::ItemType::Crab },
    { "fish/Lobster.png", "Lobster", Game::ItemType::Lobster },
    { "fish/Shrimp.png", "Shrimp", Game::ItemType::Shrimp }
};

// 判断今天是否为钓鱼节，用于切换奖励鱼种。
inline bool isFishingFestivalToday() {
    auto& ws = Game::globalState();
    return ws.seasonIndex == 1 && ws.dayOfSeason == GameConfig::FESTIVAL_DAY;
}

// 在钓鱼节当天，随机选出一条节日鱼。
inline int randomFestivalFishIndex() {
    int count = static_cast<int>(sizeof(kFestivalFish) / sizeof(kFestivalFish[0]));
    if (count <= 0) return -1;
    return std::rand() % count;
}
} // namespace

namespace Controllers {

void FishingController::buildOverlayAt(const Vec2& worldPos) {
    if (_overlay) destroyOverlay();
    if (!_scene) return;
    // 创建 UI 覆盖层并放到世界坐标附近。
    _overlay = Node::create();
    Vec2 pos = worldPos;
    if (_worldNode) pos = _worldNode->convertToWorldSpace(worldPos);
    _overlay->setPosition(pos + Vec2(220.0f, 20.0f));
    _scene->addChild(_overlay, 5);

    // Expand catch bar vertical range by 3 tiles up and down (total +6 tiles)
    _barHeight = 220.0f + 6.0f * static_cast<float>(GameConfig::TILE_SIZE);

    // Background image (Fishing game map)
    _bgSprite = Sprite::create("fish/Fishing game map.png");
    if (_bgSprite) {
        _bgSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
        _bgSprite->setPosition(Vec2(0, 0));
        auto vs = Director::getInstance()->getVisibleSize();
        // Scale to occupy ~60% of screen width/height while preserving aspect
        auto texSize = _bgSprite->getContentSize();
        float targetW = vs.width * 0.6f;
        float targetH = vs.height * 0.6f;
        float sx = targetW / texSize.width;
        float sy = targetH / texSize.height;
        float s = std::min(sx, sy);
        _bgSprite->setScale(s);
        _overlay->addChild(_bgSprite, 0);
    }

    // 绿色捕获条所在的竖直轨道背景。
    _barBg = DrawNode::create();
    _overlay->addChild(_barBg);
    _barBg->setPosition(Vec2(-static_cast<float>(GameConfig::TILE_SIZE), 0));
    float w = 36.0f; float h = _barHeight;
    Vec2 v[4] = { Vec2(-w/2,-h/2), Vec2(w/2,-h/2), Vec2(w/2,h/2), Vec2(-w/2,h/2) };

    _barCatch = DrawNode::create();
    _overlay->addChild(_barCatch);
    _progressFill = DrawNode::create();
    _overlay->addChild(_progressFill);
    // Shift vertical progress meter right by 1 tile (net left offset becomes -1.5 tiles)
    _progressFill->setPosition(Vec2(-static_cast<float>(GameConfig::TILE_SIZE) * 1.5f, 0));
    _progressLabel = Label::createWithTTF("Catch 0%", "fonts/Marker Felt.ttf", 18);
    _progressLabel->setPosition(Vec2(-static_cast<float>(GameConfig::TILE_SIZE), h/2 + 22 + 2.0f * static_cast<float>(GameConfig::TILE_SIZE)));
    _overlay->addChild(_progressLabel);

    // 选择本次小游戏中要捕获的鱼纹理：节日当天从表中随机选择。
    _festivalFishIndex = -1;
    const char* texturePath = "fish/Bream.png";
    if (isFishingFestivalToday()) {
        int idx = randomFestivalFishIndex();
        if (idx >= 0) {
            _festivalFishIndex = idx;
            texturePath = kFestivalFish[idx].texturePath;
        }
    }
    _fishSprite = Sprite::create(texturePath);
    if (_fishSprite) { _fishSprite->setScale(0.6f); _overlay->addChild(_fishSprite, 1); }

    // 注册鼠标事件：按住左键提升绿色条，松开则下落。
    _mouse = EventListenerMouse::create();
    _mouse->onMouseDown = [this](EventMouse* e){ if (!_active) return; if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) _hold = true; };
    _mouse->onMouseUp =   [this](EventMouse* e){ if (!_active) return; if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) _hold = false; };
    if (_scene) _scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_mouse, _overlay);
}

void FishingController::destroyOverlay() {
    if (_mouse && _scene) { _scene->getEventDispatcher()->removeEventListener(_mouse); }
    _mouse = nullptr;
    if (_overlay) { _overlay->removeFromParent(); _overlay = nullptr; }
    _barBg = nullptr; _barCatch = nullptr; _progressFill = nullptr; _progressLabel = nullptr; _fishSprite = nullptr;
    // Restore default height after overlay destroyed
    _barHeight = 220.0f;
}

// 在湖泊附近开始一次标准钓鱼（需要靠近水面）。
void FishingController::startAt(const Vec2& worldPos) {
    if (_active) return;
    if (_cooldown > 0.0f) {
        if (_ui) {
            float s = _map ? _map->tileSize() : static_cast<float>(GameConfig::TILE_SIZE);
            Vec2 origin = _map ? _map->getOrigin() : Vec2::ZERO;
            Vec2 head = origin + worldPos + Vec2(0, s);
            _ui->popTextAt(head, "Need a short break", Color3B::YELLOW);
        }
        return;
    }
    float s = _map ? _map->tileSize() : static_cast<float>(GameConfig::TILE_SIZE);
    float radius = s * GameConfig::LAKE_REFILL_RADIUS_TILES;
    if (_map && !_map->isNearLake(worldPos, radius)) {
        return;
    }
    _active = true;
    _progress = 0.0f;
    _timeLeft = 12.0f;
    _barCatchPos = 80.0f;
    _barCatchVel = 0.0f;
    _fishPos = 140.0f;
    _fishVel = 0.0f;
    _hold = false;
    buildOverlayAt(worldPos);
    if (_overlay && _scene) {
        auto vs = cocos2d::Director::getInstance()->getVisibleSize();
        auto org = cocos2d::Director::getInstance()->getVisibleOrigin();
        _overlay->setPosition(org + cocos2d::Vec2(vs.width * 0.68f, vs.height * 0.55f));
    }
    if (_setMovementLocked) { _setMovementLocked(true); }
    Game::globalState().fishingActive = true;
}

// 任意位置开始钓鱼小游戏，主要用于节日等不受限制的场景。
void FishingController::startAnywhere(const Vec2& worldPos) {
    if (_active) return;
    if (_cooldown > 0.0f) {
        if (_ui) {
            float s = _map ? _map->tileSize() : static_cast<float>(GameConfig::TILE_SIZE);
            Vec2 origin = _map ? _map->getOrigin() : Vec2::ZERO;
            Vec2 head = origin + worldPos + Vec2(0, s);
            _ui->popTextAt(head, "Need a short break", Color3B::YELLOW);
        }
        return;
    }
    _active = true;
    _progress = 0.0f;
    _timeLeft = 12.0f;
    _barCatchPos = 80.0f;
    _barCatchVel = 0.0f;
    _fishPos = 140.0f;
    _fishVel = 0.0f;
    _hold = false;
    buildOverlayAt(worldPos);
    if (_overlay && _scene) {
        auto vs = cocos2d::Director::getInstance()->getVisibleSize();
        auto org = cocos2d::Director::getInstance()->getVisibleOrigin();
        _overlay->setPosition(org + cocos2d::Vec2(vs.width * 0.68f, vs.height * 0.55f));
    }
    if (_setMovementLocked) { _setMovementLocked(true); }
    Game::globalState().fishingActive = true;
}

void FishingController::update(float dt) {
    // 先更新冷却计时；冷却中直接返回不处理逻辑。
    if (_cooldown > 0.0f) {
        _cooldown -= dt;
        if (_cooldown < 0.0f) _cooldown = 0.0f;
    }
    if (!_active) return;
    // 绿色捕获条的“受力”模拟：按住时上升，松开时下落，并带阻尼。
    float accelUp = 240.0f; float gravity = 280.0f; float damping = 0.98f;
    _barCatchVel += (_hold ? accelUp : -gravity) * dt;
    _barCatchVel *= damping;
    _barCatchPos += _barCatchVel * dt;
    if (_barCatchPos < 0) { _barCatchPos = 0; _barCatchVel = 0; }
    if (_barCatchPos > _barHeight) { _barCatchPos = _barHeight; _barCatchVel = 0; }

    // 鱼在轨道上的随机游动，带有边界弹回。
    float fishAccel = ((std::rand() % 200) - 100) * 4.0f;
    _fishVel += fishAccel * dt;
    _fishVel = std::max(-200.0f, std::min(200.0f, _fishVel));
    _fishPos += _fishVel * dt;
    if (_fishPos < 20) { _fishPos = 20; _fishVel = std::abs(_fishVel); }
    if (_fishPos > _barHeight - 20) { _fishPos = _barHeight - 20; _fishVel = -std::abs(_fishVel); }

    // 统计绿色条与鱼的重叠程度，决定“捕获进度”的增减。
    float catchSize = 54.0f; // green bar height
    float diff = std::abs((_barCatchPos) - (_fishPos));
    bool overlap = diff <= catchSize * 0.5f;
    float gain = overlap ? 28.0f : -18.0f;
    _progress = std::max(0.0f, std::min(100.0f, _progress + gain * dt));
    _timeLeft -= dt;

    // 绘制捕获条、进度条以及鱼的当前位置。
    if (_barCatch) {
        _barCatch->clear();
        float w = 28.0f; float h = catchSize;
        Vec2 v[4] = { Vec2(-w/2, -h/2), Vec2(w/2, -h/2), Vec2(w/2, h/2), Vec2(-w/2, h/2) };
        _barCatch->drawSolidPoly(v, 4, Color4F(0.15f, 0.85f, 0.35f, 0.9f));
        _barCatch->setPosition(Vec2(-static_cast<float>(GameConfig::TILE_SIZE), _barCatchPos - _barHeight*0.5f));
    }
    if (_progressFill) {
        _progressFill->clear();
        // Extend meter by 2 tiles up and 2 tiles down (total +4 tiles)
        float extra = static_cast<float>(GameConfig::TILE_SIZE) * 2.0f;
        float pv = _barHeight + extra * 2.0f;   // extended total height
        float pw = 12.0f;                // meter width
        float ratio = _progress / 100.0f;
        float fillH = pv * ratio;        // fill respects extended range
        float x = 40.0f;                 // place to the right of track center
        float yBottom = -_barHeight * 0.5f - extra; // push bottom down by 2 tiles
        Vec2 bl(x, yBottom);
        Vec2 br(x + pw, yBottom);
        Vec2 tr(x + pw, yBottom + fillH);
        Vec2 tl(x, yBottom + fillH);
        Vec2 rect[4] = { bl, br, tr, tl };
        _progressFill->drawSolidPoly(rect, 4, Color4F(0.2f, 0.6f, 1.0f, 0.85f));
    }
    if (_progressLabel) {
        _progressLabel->setString(StringUtils::format("Catch %d%%", static_cast<int>(_progress + 0.5f)));
    }
    if (_fishSprite) {
        _fishSprite->setPosition(Vec2(-static_cast<float>(GameConfig::TILE_SIZE), _fishPos - _barHeight*0.5f));
    }

    // 达到 100% 视为成功；时间耗尽视为失败。
        Vec2 posWorld = _worldNode ? _worldNode->convertToNodeSpace(_overlay->getPosition()) : _overlay->getPosition();
        onSuccess(posWorld);
    } else if (_timeLeft <= 0.0f) {
        Vec2 posWorld = _worldNode ? _worldNode->convertToNodeSpace(_overlay->getPosition()) : _overlay->getPosition();
        onFail(posWorld);
    }
}

// 钓鱼成功：奖励鱼类、增加经验并播放提示。
void FishingController::onSuccess(const Vec2& worldPos) {
    _active = false;
    destroyOverlay();
    if (_setMovementLocked) { _setMovementLocked(false); }
    if (_inventory) {
        auto& skill = Game::SkillTreeSystem::getInstance();
        int qty = skill.adjustFishCatchQuantityForFishing(1);
        Game::ItemType itemType = Game::ItemType::BreamFish;
        // 钓鱼节当天改为节日鱼，并在提示文字中显示鱼名。
        if (isFishingFestivalToday() && _festivalFishIndex >= 0) {
            int count = static_cast<int>(sizeof(kFestivalFish) / sizeof(kFestivalFish[0]));
            if (_festivalFishIndex < count) {
                itemType = kFestivalFish[_festivalFishIndex].itemType;
            }
        }
        _inventory->addItems(itemType, qty);
        skill.addXp(Game::SkillTreeType::Fishing, skill.xpForFishingCatch(qty));
        if (_ui && _ui->isSkillTreePanelVisible()) {
            _ui->refreshSkillTreePanel();
        }
    }
    Game::globalState().fishingActive = false;
    _cooldown = 2.0f;
    if (_ui) {
        _ui->refreshHotbar();
        std::string text = "Caught a fish!";
        if (isFishingFestivalToday() && _festivalFishIndex >= 0) {
            int count = static_cast<int>(sizeof(kFestivalFish) / sizeof(kFestivalFish[0]));
            if (_festivalFishIndex < count) {
                text = StringUtils::format("Caught %s!", kFestivalFish[_festivalFishIndex].displayName);
            }
        }
        _ui->popTextAt(worldPos, text, Color3B::GREEN);
    }
}

// 钓鱼失败：只重置状态并给出“逃跑”提示。
void FishingController::onFail(const Vec2& worldPos) {
    _active = false;
    destroyOverlay();
    if (_setMovementLocked) { _setMovementLocked(false); }
    _cooldown = 2.0f;
    Game::globalState().fishingActive = false;
    if (_ui) {
        _ui->popTextAt(worldPos, "Fish escaped", Color3B::RED);
    }
}

// 主动取消当前钓鱼过程（例如玩家切换场景时调用）。
void FishingController::cancel() {
    if (!_active) return;
    _active = false;
    destroyOverlay();
    if (_setMovementLocked) { _setMovementLocked(false); }
    _cooldown = 1.0f;
    Game::globalState().fishingActive = false;
}
// namespace Controllers
}
