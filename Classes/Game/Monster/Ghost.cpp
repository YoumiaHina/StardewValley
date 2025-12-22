#include "Game/Monster/MonsterBase.h"
#include "cocos2d.h"
#include <cmath>

namespace Game {

namespace {

// 根据幽灵当前速度向量，决定使用贴图中的哪一行：
// - 速度接近 0 认为静止，固定选第 0 行（第一排）
// - 横向分量更大时使用左右行（第 1/3 行）
// - 纵向分量更大时使用上下行（第 0/2 行）
int ghostRowForVelocity(const Monster& m) {
    cocos2d::Vec2 v = m.velocity;
    float len2 = v.x * v.x + v.y * v.y;
    if (len2 <= 1e-4f) return 4;
    float ax = std::fabs(v.x);
    float ay = std::fabs(v.y);
    if (ax >= ay) {
        // 向右使用第 1 行（第二排），向左使用第 3 行（第四排）
        return v.x >= 0.0f ? 3 : 1;
    } else {
        // 向上使用第 2 行（第三排），向下依然使用第 0 行（第一排）
        return v.y >= 0.0f ? 2 : 4;
    }
}

// 按给定的行号，从 GhostMove 贴图中切出该行的 4 帧动画
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
        // Cocos2d 纹理原点在左下，贴图按行从上到下排布，这里通过 texH - frameH*(row+1) 换算出第 row 行
        float y = texH - frameH * static_cast<float>(row + 1);
        // 这里选中的是第 row 行第 col 列的格子
        auto frame = cocos2d::SpriteFrame::create("Monster/GhostMove.png", cocos2d::Rect(x, y, frameW, frameH));
        if (!frame) continue;
        anim->addSpriteFrame(frame);
    }
    // 幽灵动作偏快，这里每帧时间可以根据需要调整
    anim->setDelayPerUnit(0.06f);
    return anim;
}

const int kGhostAnimActionTag = 9101;

// 在指定行上循环播放幽灵的移动/待机动画：
// - 如果已经在播同一行，则复用现有动作
// - 行号通过 sprite 的 tag 记录，动作通过统一 actionTag 管理
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

// Ghost 幽灵怪物的行为与动画实现：
// - 使用 GhostMove.png，并根据方向映射到指定的行
//   第 0 行：静止/向下
//   第 1 行：向右
//   第 2 行：向上
//   第 3 行：向左
//   第 4 行：死亡动画
class GhostMonster : public MonsterBase {
public:
    // 初始化幽灵基础属性与掉落配置
    GhostMonster() {
        def_.hp = 80;
        def_.dmg = 15;
        def_.def = 2;
        def_.searchRangeTiles = 20;
        def_.moveSpeed = 100.0f;
        def_.isCollisionAffected = false;
        def_.name = "Ghost";
        drops_.push_back(ItemType::Coal);
    }

    // 返回怪物类型枚举，用于系统层区分怪物种类
    Monster::Type monsterType() const override {
        return Monster::Type::Ghost;
    }

    // 静止时循环播放第 0 行（第一排）的 4 帧动画
    void playStaticAnimation(const Monster& m, cocos2d::Sprite* sprite) const override {
        ghostRunLoopRow(4, sprite);
    }

    // 移动时根据速度方向选择行号，再循环播放该行的 4 帧动画
    // - 向下：第 0 行（第一排）
    // - 向右：第 1 行（第二排）
    // - 向上：第 2 行（第三排）
    // - 向左：第 3 行（第四排）
    void playMoveAnimation(const Monster& m, cocos2d::Sprite* sprite) const override {
        int row = ghostRowForVelocity(m);
        ghostRunLoopRow(row, sprite);
    }

    // 死亡时循环播放第 0 行（第一排）的 4 帧动画，然后淡出并回调通知系统移除
    void playDeathAnimation(const Monster& m, cocos2d::Sprite* sprite, const std::function<void()>& onComplete) const override {
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

        // 这里选取死亡动画所在的第 0 行（第一排）的 4 张图来做循环
        auto anim = cocos2d::Animation::create();
        if (!anim) {
            if (onComplete) onComplete();
            return;
        }
        for (int col = 0; col < 4; ++col) {
            float x = frameW * static_cast<float>(col);
            // 第 0 行：row = 0，对应第一排
            float y = texH - frameH * static_cast<float>(0 + 1);
            // 这里选中的是死亡行（第 0 行）第 col 列的格子
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
                // 动画播完并淡出后再移除幽灵
                if (onComplete) onComplete();
            }),
            nullptr);
        sprite->runAction(seq);
    }
};

const MonsterBase& ghostMonsterBehavior() {
    static GhostMonster inst;
    return inst;
}

} // namespace Game
