#include "Game/Monster/MonsterBase.h"
#include "cocos2d.h"
#include <cmath>

namespace Game {

namespace {

// ghostRowForVelocity：根据速度方向选择 GhostMove.png 中的行号。
// - 逻辑与 Bug 类似，只是行号映射略有不同。
int ghostRowForVelocity(const cocos2d::Vec2& v) {
    float len2 = v.x * v.x + v.y * v.y;
    if (len2 <= 1e-4f) return 4; // 几乎不动：使用第 4 行。
    float ax = std::fabs(v.x);
    float ay = std::fabs(v.y);
    if (ax >= ay) {
        // 向右使用第 1 行（第二排），向左使用第 3 行（第四排）。
        return v.x >= 0.0f ? 3 : 1;
    } else {
        // 向上使用第 2 行（第三排），向下依然使用第 0 行（第一排）。
        return v.y >= 0.0f ? 2 : 4;
    }
}

// 按给定的行号，从 GhostMove 贴图中切出该行的 4 帧动画。
cocos2d::Animation* ghostRowAnimation(int row) {
    auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/GhostMove.png");
    if (!tex) return nullptr;
    tex->setAliasTexParameters();
    auto size = tex->getContentSize();
    if (size.width <= 0.0f || size.height <= 0.0f) return nullptr;
    float frameW = size.width / 4.0f;
    float frameH = size.height / 5.0f;
    float texH = size.height;
    auto anim = cocos2d::Animation::create();
    if (!anim) return nullptr;
    for (int col = 0; col < 4; ++col) {
        float x = frameW * static_cast<float>(col);
        // Cocos2d 纹理原点在左下，贴图按行从上到下排布，这里通过 texH - frameH*(row+1) 换算出第 row 行。
        float y = texH - frameH * static_cast<float>(row + 1);
        // 这里选中的是第 row 行第 col 列的格子。
        auto frame = cocos2d::SpriteFrame::create("Monster/GhostMove.png", cocos2d::Rect(x, y, frameW, frameH));
        if (!frame) continue;
        anim->addSpriteFrame(frame);
    }
    // 幽灵动作偏快，这里每帧时间可以根据需要调整。
    anim->setDelayPerUnit(0.06f);
    return anim;
}

const int kGhostAnimActionTag = 9101;

// ghostRunLoopRow：在指定行上循环播放幽灵的移动/待机动画。
// - 如果已经在播同一行，则复用现有动作；
// - 精灵的 tag 字段记录当前行号，动作通过统一 actionTag 管理。
void ghostRunLoopRow(int row, cocos2d::Sprite* sprite) {
    if (!sprite) return;
    int currentRow = sprite->getTag();
    auto running = sprite->getActionByTag(kGhostAnimActionTag);
    if (running && currentRow == row) return;
    sprite->stopActionByTag(kGhostAnimActionTag);
    auto anim = ghostRowAnimation(row);
    if (!anim) return;
    sprite->setTag(row);
    auto act = cocos2d::RepeatForever::create(cocos2d::Animate::create(anim));
    act->setTag(kGhostAnimActionTag);
    sprite->runAction(act);
}

} // namespace

// GhostMonster：幽灵怪物的行为与动画实现。
// 使用 GhostMove.png，并根据方向映射到指定的行：
//   第 0 行：静止/向下
//   第 1 行：向右
//   第 2 行：向上
//   第 3 行：向左
//   第 4 行：待机/特殊状态
class GhostMonster : public MonsterBase {
public:
    // 初始化幽灵基础属性与掉落配置。
    GhostMonster() {
        def_.hp = 80;
        def_.dmg = 15;
        def_.def = 2;
        def_.searchRangeTiles = 20;
        def_.moveSpeed = 100.0f;
        def_.isCollisionAffected = false; // 幽灵不受碰撞影响，可以穿墙。
        def_.name = "Ghost";
        drops_.push_back(ItemType::Coal);
    }

    MonsterType monsterType() const override {
        return MonsterType::Ghost;
    }

    // 静止动画：这里选择第 4 行作为“待机”行。
    void playStaticAnimation(cocos2d::Sprite* sprite) const override {
        ghostRunLoopRow(4, sprite);
    }

    // 移动时根据速度方向选择行号，再循环播放该行的 4 帧动画。
    void playMoveAnimation(const cocos2d::Vec2& velocity, cocos2d::Sprite* sprite) const override {
        int row = ghostRowForVelocity(velocity);
        ghostRunLoopRow(row, sprite);
    }

    // 死亡动画：从第 0 行（第一排）裁剪 4 帧当作死亡动画，播放一次后做淡出。
    void playDeathAnimation(cocos2d::Sprite* sprite, const std::function<void()>& onComplete) const override {
        if (!sprite) {
            if (onComplete) onComplete();
            return;
        }
        auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/GhostMove.png");
        if (!tex) {
            if (onComplete) onComplete();
            return;
        }
        tex->setAliasTexParameters();
        auto size = tex->getContentSize();
        if (size.width <= 0.0f || size.height <= 0.0f) {
            if (onComplete) onComplete();
            return;
        }
        float frameW = size.width / 4.0f;
        float frameH = size.height / 5.0f;
        float texH = size.height;

        // 这里选取死亡动画所在的第 0 行（第一排）的 4 张图来做循环。
        auto anim = cocos2d::Animation::create();
        if (!anim) {
            if (onComplete) onComplete();
            return;
        }
        for (int col = 0; col < 4; ++col) {
            float x = frameW * static_cast<float>(col);
            float y = texH - frameH * static_cast<float>(0 + 1);
            auto frame = cocos2d::SpriteFrame::create("Monster/GhostMove.png", cocos2d::Rect(x, y, frameW, frameH));
            if (!frame) continue;
            anim->addSpriteFrame(frame);
        }
        anim->setDelayPerUnit(0.08f);

        sprite->stopActionByTag(kGhostAnimActionTag);
        sprite->setTexture("Monster/GhostMove.png");
        sprite->setOpacity(255);
        auto act = cocos2d::Animate::create(anim);
        auto fade = cocos2d::FadeOut::create(0.25f);
        auto seq = cocos2d::Sequence::create(
            act,
            fade,
            cocos2d::CallFunc::create([onComplete]() {
                // 动画播完并淡出后再移除幽灵。
                if (onComplete) onComplete();
            }),
            nullptr);
        sprite->runAction(seq);
    }
};

// ghostMonsterBehavior：返回静态行为对象引用。
const MonsterBase& ghostMonsterBehavior() {
    static GhostMonster inst;
    return inst;
}

} // namespace Game
