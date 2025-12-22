#pragma once

#include "Game/Monster/MonsterBase.h"
#include "cocos2d.h"

namespace Game {

// 史莱姆怪物基类：
// - 继承自 MonsterBase，封装三色史莱姆共享的动画与行为逻辑
// - 子类只需要负责数值配置与调色（提供不同的主体颜色）
class SlimeMonsterBase : public MonsterBase {
public:
    virtual ~SlimeMonsterBase() = default;

    // 返回当前史莱姆的主体颜色，由三色子类实现
    virtual cocos2d::Color3B slimeColor() const = 0;

    // 静止动画：高频率循环播放 RockSlimeMove.png 中第 kSlimeAnimStartRow~kSlimeAnimEndRow 行的所有贴图
    void playStaticAnimation(const Monster& m, cocos2d::Sprite* sprite) const override {
        if (!sprite) return;
        slimeRunCommonLoop(sprite);
        sprite->setColor(slimeColor());
    }

    // 移动动画：与静止相同，也采用高频率循环播放 RockSlimeMove.png 中第 kSlimeAnimStartRow~kSlimeAnimEndRow 行
    // - 如需区分行走/待机，可以在此处改为调用不同的辅助函数
    void playMoveAnimation(const Monster& m, cocos2d::Sprite* sprite) const override {
        if (!sprite) return;
        slimeRunCommonLoop(sprite);
        sprite->setColor(slimeColor());
    }

    // 死亡动画：使用 RockSlimeDying.png 这张完整图片做淡出效果
    void playDeathAnimation(const Monster& m, cocos2d::Sprite* sprite, const std::function<void()>& onComplete) const override {
        if (!sprite) {
            if (onComplete) onComplete();
            return;
        }
        auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/RockSlimeDying.png");
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
        sprite->stopActionByTag(kSlimeAnimActionTag);
        // 停止并移除眼睛覆盖动画
        sprite->stopActionByTag(kSlimeEyesActionTag);
        if (auto eyes = sprite->getChildByTag(kSlimeEyesSpriteTag)) {
            eyes->removeFromParent();
        }
        sprite->setTexture("Monster/RockSlimeDying.png");
        // RockSlimeDying.png 是单张死亡图片，这里使用整张纹理区域
        sprite->setTextureRect(cocos2d::Rect(0.0f, 0.0f, size.width, size.height));
        sprite->setColor(slimeColor());
        sprite->setOpacity(255);
        // 仅做淡出效果作为死亡动画
        auto fade = cocos2d::FadeOut::create(0.25f);
        auto seq = cocos2d::Sequence::create(
            fade,
            cocos2d::CallFunc::create([onComplete]() {
                // 淡出完成后再执行销毁回调
                if (onComplete) onComplete();
            }),
            nullptr);
        sprite->runAction(seq);
    }

protected:
    // 公共活动动画使用的行区间（含首尾），对 RockSlimeMove.png 默认使用全部 0~4 行
    static constexpr int kSlimeAnimStartRow  = 0;  // 活动动画起始行号（包含）
    static constexpr int kSlimeAnimEndRow    = 4;  // 活动动画结束行号（包含）

    static const int kSlimeAnimActionTag = 9201;
    static const int kSlimeEyesActionTag = 9202;
    static const int kSlimeEyesSpriteTag = 9301;

    // 按给定的行号，从 RockSlimeMove.png 中切出该行的 4 帧动画
    cocos2d::Animation* slimeRowAnimation(int row) const {
        auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/RockSlimeMove.png");
        if (!tex) return nullptr;
        tex->setAliasTexParameters();
        auto size = tex->getContentSize();
        if (size.width <= 0.0f || size.height <= 0.0f) return nullptr;
        float frameW = size.width / 4.0f;
        float frameH = size.height / 5.0f; // 5 行 4 列，每小图约宽 16 高 24 像素
        float texH = size.height;
        auto anim = cocos2d::Animation::create();
        if (!anim) return nullptr;
        for (int col = 0; col < 4; ++col) {
            float x = frameW * static_cast<float>(col);
            // Cocos2d 纹理原点在左下，这里通过 texH - frameH*(row+1) 换算出第 row 行
            float y = texH - frameH * static_cast<float>(row + 1);
            // 这里选中的是第 row 行第 col 列的格子
            auto frame = cocos2d::SpriteFrame::create("Monster/RockSlimeMove.png", cocos2d::Rect(x, y, frameW, frameH));
            if (!frame) continue;
            anim->addSpriteFrame(frame);
        }
        anim->setDelayPerUnit(0.08f);
        return anim;
    }

    // 生成史莱姆公共活动动画：遍历第 kSlimeAnimStartRow~kSlimeAnimEndRow 行的所有贴图
    cocos2d::Animation* slimeCommonAnimation() const {
        auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/RockSlimeMove.png");
        if (!tex) return nullptr;
        tex->setAliasTexParameters();
        auto size = tex->getContentSize();
        if (size.width <= 0.0f || size.height <= 0.0f) return nullptr;
        float frameW = size.width / 4.0f;
        float frameH = size.height / 5.0f; // RockSlimeMove.png：5 行 4 列，每帧约宽 16 高 24 像素
        float texH = size.height;
        auto anim = cocos2d::Animation::create();
        if (!anim) return nullptr;
        for (int row = kSlimeAnimStartRow; row <= kSlimeAnimEndRow; ++row) {
            for (int col = 0; col < 4; ++col) {
                float x = frameW * static_cast<float>(col);
                float y = texH - frameH * static_cast<float>(row + 1);
                // 这里选中的是 RockSlimeMove.png 第 row 行第 col 列的格子
                auto frame = cocos2d::SpriteFrame::create("Monster/RockSlimeMove.png", cocos2d::Rect(x, y, frameW, frameH));
                if (!frame) continue;
                anim->addSpriteFrame(frame);
            }
        }
        // 高频率播放：可以根据需要继续调小
        anim->setDelayPerUnit(0.04f);
        return anim;
    }

    // 在第 kSlimeAnimStartRow~kSlimeAnimEndRow 行上循环播放史莱姆的活动动画，并叠加眼睛动画
    void slimeRunCommonLoop(cocos2d::Sprite* sprite) const {
        if (!sprite) return;
        auto running = sprite->getActionByTag(kSlimeAnimActionTag);
        if (running) return;
        sprite->stopActionByTag(kSlimeAnimActionTag);
        auto anim = slimeCommonAnimation();
        if (!anim) return;
        auto act = cocos2d::RepeatForever::create(cocos2d::Animate::create(anim));
        act->setTag(kSlimeAnimActionTag);
        sprite->runAction(act);
        // 在身体动画之上叠加眼睛覆盖动画
        slimeRunEyesLoop(sprite);
    }

    // 生成史莱姆眼睛动画：从 RockSlimeEyes.png 中拆分 4 帧（每帧宽 16 高 24）
    cocos2d::Animation* slimeEyesAnimation() const {
        auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/RockSlimeEyes.png");
        if (!tex) return nullptr;
        tex->setAliasTexParameters();
        auto size = tex->getContentSize();
        if (size.width <= 0.0f || size.height <= 0.0f) return nullptr;
        // 假定 RockSlimeEyes.png 为 2 行 2 列：整体宽 32 高 48，每小格 16x24，共 4 帧
        float frameW = size.width / 2.0f;   // 每帧宽 16
        float frameH = size.height / 2.0f;  // 每帧高 24
        float texH = size.height;
        auto anim = cocos2d::Animation::create();
        if (!anim) return nullptr;
        for (int i = 0; i < 4; ++i) {
            int row = i / 2;
            int col = i % 2;
            float x = frameW * static_cast<float>(col);
            float y = texH - frameH * static_cast<float>(row + 1);
            // 这里选中的是 RockSlimeEyes.png 第 row 行第 col 列的眼睛格子
            auto frame = cocos2d::SpriteFrame::create("Monster/RockSlimeEyes.png",
                                                      cocos2d::Rect(x, y, frameW, frameH));
            if (!frame) continue;
            anim->addSpriteFrame(frame);
        }
        anim->setDelayPerUnit(1.0f);
        return anim;
    }

    // 在史莱姆身体精灵上创建并循环播放眼睛覆盖动画
    void slimeRunEyesLoop(cocos2d::Sprite* body) const {
        if (!body) return;
        auto eyes = dynamic_cast<cocos2d::Sprite*>(body->getChildByTag(kSlimeEyesSpriteTag));
        if (!eyes) {
            auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/RockSlimeEyes.png");
            if (!tex) return;
            tex->setAliasTexParameters();
            auto size = tex->getContentSize();
            if (size.width <= 0.0f || size.height <= 0.0f) return;
            // 与 slimeEyesAnimation 相同：2 行 2 列，每帧 16x24
            float frameW = size.width / 2.0f;
            float frameH = size.height / 2.0f;
            float texH = size.height;
            eyes = cocos2d::Sprite::create();
            if (!eyes) return;
            eyes->setTexture(tex);
            // 初始显示第 0 帧的眼睛（第 0 行第 0 列）
            float x0 = 0.0f;
            float y0 = texH - frameH * static_cast<float>(0 + 1);
            eyes->setTextureRect(cocos2d::Rect(x0, y0, frameW, frameH));
            // 眼睛锚点使用中心，方便与身体相对偏移对齐
            eyes->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
            // 身体小图高度约为 24，这里将眼睛中心略微向右上偏移
            float bodyWidth = 16.0f;
            float bodyHeight = 24.0f;
            eyes->setPosition(cocos2d::Vec2(bodyWidth * 0.5f, bodyHeight * 0.6f));
            eyes->setTag(kSlimeEyesSpriteTag);
            body->addChild(eyes, 1);
        }
        auto running = eyes->getActionByTag(kSlimeEyesActionTag);
        if (running) return;
        auto anim = slimeEyesAnimation();
        if (!anim) return;
        auto act = cocos2d::RepeatForever::create(cocos2d::Animate::create(anim));
        act->setTag(kSlimeEyesActionTag);
        eyes->runAction(act);
    }
};

} // namespace Game
