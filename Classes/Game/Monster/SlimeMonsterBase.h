#pragma once

#include "Game/Monster/MonsterBase.h"
#include "cocos2d.h"

namespace Game {

// SlimeMonsterBase：史莱姆怪物的“行为基类”。
// - 继承自 MonsterBase，封装三种史莱姆（绿/蓝/红）的公共动画逻辑；
// - 子类只需要提供：
//   1）构造函数里填好 def_（血量、攻击力等）和 drops_（掉落列表）；
//   2）重写 slimeColor() 返回不同颜色；
// - 这样可以避免在每个史莱姆 cpp 里复制大段动画代码。
class SlimeMonsterBase : public MonsterBase {
public:
    // 虚析构函数：允许通过 MonsterBase 指针安全删除 SlimeMonsterBase。
    virtual ~SlimeMonsterBase() = default;

    // 纯虚函数：每个子类必须给出一个颜色，用于给 sprite 调色。
    virtual cocos2d::Color3B slimeColor() const = 0;

    // 静止动画：这里直接复用“公共活动动画”，只是速度较快。
    // - 接口来自 MonsterBase，使用 override 关键字表示“重写父类虚函数”；
    // - override 可以让编译器帮我们检查函数签名是否匹配。
    void playStaticAnimation(cocos2d::Sprite* sprite) const override {
        if (!sprite) return;              // 空指针保护：如果精灵为空，什么也不做。
        slimeRunCommonLoop(sprite);       // 播放共用的循环动画。
        sprite->setColor(slimeColor());   // 调整精灵颜色，让绿史莱姆/红史莱姆看起来不同。
    }

    // 移动动画：当前实现与静止动画完全相同，同样调用 slimeRunCommonLoop。
    // - 以后如果想区分“待机”和“移动”的行，可以在此处改为根据速度选行。
    void playMoveAnimation(const cocos2d::Vec2& velocity, cocos2d::Sprite* sprite) const override {
        (void)velocity;                   // 当前未使用速度，只是为了符合接口，避免编译告警。
        if (!sprite) return;
        slimeRunCommonLoop(sprite);
        sprite->setColor(slimeColor());
    }

    // 死亡动画：切换到 RockSlimeDying.png 这一张“死亡贴图”，并做淡出效果。
    // - onComplete 回调在淡出结束后调用，用于通知外部“可以销毁怪物节点了”。
    void playDeathAnimation(cocos2d::Sprite* sprite, const std::function<void()>& onComplete) const override {
        if (!sprite) {
            // 如果没有传入有效 sprite，就直接调用回调，避免上层逻辑卡死。
            if (onComplete) onComplete();
            return;
        }
        auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/RockSlimeDying.png");
        if (!tex) {
            if (onComplete) onComplete();
            return;
        }
        // setAliasTexParameters：关闭纹理的“模糊过滤”，避免像素风贴图被放大时变糊。
        tex->setAliasTexParameters();
        auto size = tex->getContentSize();
        if (size.width <= 0.0f || size.height <= 0.0f) {
            if (onComplete) onComplete();
            return;
        }
        // 停止身体动画和眼睛动画，避免死亡时还在动。
        sprite->stopActionByTag(kSlimeAnimActionTag);
        sprite->stopActionByTag(kSlimeEyesActionTag);
        if (auto eyes = sprite->getChildByTag(kSlimeEyesSpriteTag)) {
            eyes->removeFromParent();
        }
        // 将精灵纹理切换为 RockSlimeDying.png 整张图。
        sprite->setTexture("Monster/RockSlimeDying.png");
        sprite->setTextureRect(cocos2d::Rect(0.0f, 0.0f, size.width, size.height));
        sprite->setColor(slimeColor());
        sprite->setOpacity(255);
        // 使用 cocos2d::FadeOut 创建 0.25 秒的淡出动作。
        auto fade = cocos2d::FadeOut::create(0.25f);
        // Sequence：顺序执行一系列动作，最后调用一个回调。
        auto seq = cocos2d::Sequence::create(
            fade,
            cocos2d::CallFunc::create([onComplete]() {
                // 淡出完成后再执行销毁回调。
                if (onComplete) onComplete();
            }),
            nullptr);
        sprite->runAction(seq);
    }

protected:
    // 公共活动动画使用的行区间（含首尾），对 RockSlimeMove.png 默认使用全部 0~4 行。
    // static constexpr：编译期常量，所有实例共享，相当于 C 里的常量宏，但有类型。
    static constexpr int kSlimeAnimStartRow  = 0;  // 活动动画起始行号（包含）
    static constexpr int kSlimeAnimEndRow    = 4;  // 活动动画结束行号（包含）

    // 这些“Tag”用于给 Action 或子节点打标签，方便后续停止/查找。
    static const int kSlimeAnimActionTag = 9201;
    static const int kSlimeEyesActionTag = 9202;
    static const int kSlimeEyesSpriteTag = 9301;

    // slimeRowAnimation：按给定的行号，从 RockSlimeMove.png 中切出该行的 4 帧动画。
    // - cocos2d::Animation：一组连续播放的 SpriteFrame；
    // - 返回值是指针，失败时返回 nullptr（调用方需要判空）。
    cocos2d::Animation* slimeRowAnimation(int row) const {
        auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/RockSlimeMove.png");
        if (!tex) return nullptr;
        tex->setAliasTexParameters();
        auto size = tex->getContentSize();
        if (size.width <= 0.0f || size.height <= 0.0f) return nullptr;
        float frameW = size.width / 4.0f; // 每行 4 列小图。
        float frameH = size.height / 5.0f; // 5 行 4 列，每小图约宽 16 高 24 像素。
        float texH = size.height;
        auto anim = cocos2d::Animation::create();
        if (!anim) return nullptr;
        for (int col = 0; col < 4; ++col) {
            float x = frameW * static_cast<float>(col);
            // 注意：cocos2d 的纹理坐标原点在左下，所以要用 texH - frameH*(row+1) 计算 y。
            float y = texH - frameH * static_cast<float>(row + 1);
            auto frame = cocos2d::SpriteFrame::create("Monster/RockSlimeMove.png", cocos2d::Rect(x, y, frameW, frameH));
            if (!frame) continue;
            anim->addSpriteFrame(frame);
        }
        anim->setDelayPerUnit(0.08f); // 每帧 0.08 秒，可根据需要调整。
        return anim;
    }

    // slimeCommonAnimation：遍历 kSlimeAnimStartRow~kSlimeAnimEndRow 所有行，拼成一个“活动动画”。
    cocos2d::Animation* slimeCommonAnimation() const {
        auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/RockSlimeMove.png");
        if (!tex) return nullptr;
        tex->setAliasTexParameters();
        auto size = tex->getContentSize();
        if (size.width <= 0.0f || size.height <= 0.0f) return nullptr;
        float frameW = size.width / 4.0f;
        float frameH = size.height / 5.0f; // RockSlimeMove.png：5 行 4 列，每帧约宽 16 高 24 像素。
        float texH = size.height;
        auto anim = cocos2d::Animation::create();
        if (!anim) return nullptr;
        for (int row = kSlimeAnimStartRow; row <= kSlimeAnimEndRow; ++row) {
            for (int col = 0; col < 4; ++col) {
                float x = frameW * static_cast<float>(col);
                float y = texH - frameH * static_cast<float>(row + 1);
                auto frame = cocos2d::SpriteFrame::create("Monster/RockSlimeMove.png", cocos2d::Rect(x, y, frameW, frameH));
                if (!frame) continue;
                anim->addSpriteFrame(frame);
            }
        }
        // 高频率播放：delay 越小，动画越快。
        anim->setDelayPerUnit(0.04f);
        return anim;
    }

    // slimeRunCommonLoop：在精灵上循环播放“活动动画”，并叠加眼睛动画。
    void slimeRunCommonLoop(cocos2d::Sprite* sprite) const {
        if (!sprite) return;
        // getActionByTag：检查是否已有同一 Tag 的动作在跑，避免重复添加。
        auto running = sprite->getActionByTag(kSlimeAnimActionTag);
        if (running) return;
        sprite->stopActionByTag(kSlimeAnimActionTag);
        auto anim = slimeCommonAnimation();
        if (!anim) return;
        auto act = cocos2d::RepeatForever::create(cocos2d::Animate::create(anim));
        act->setTag(kSlimeAnimActionTag);
        sprite->runAction(act);
        // 在身体动画之上叠加眼睛覆盖动画。
        slimeRunEyesLoop(sprite);
    }

    // slimeEyesAnimation：从 RockSlimeEyes.png 拆分 4 帧眼睛动画。
    cocos2d::Animation* slimeEyesAnimation() const {
        auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/RockSlimeEyes.png");
        if (!tex) return nullptr;
        tex->setAliasTexParameters();
        auto size = tex->getContentSize();
        if (size.width <= 0.0f || size.height <= 0.0f) return nullptr;
        // 假定 RockSlimeEyes.png 为 2 行 2 列：整体宽 32 高 48，每小格 16x24，共 4 帧。
        float frameW = size.width / 2.0f;   // 每帧宽 16。
        float frameH = size.height / 2.0f;  // 每帧高 24。
        float texH = size.height;
        auto anim = cocos2d::Animation::create();
        if (!anim) return nullptr;
        for (int i = 0; i < 4; ++i) {
            int row = i / 2;
            int col = i % 2;
            float x = frameW * static_cast<float>(col);
            float y = texH - frameH * static_cast<float>(row + 1);
            auto frame = cocos2d::SpriteFrame::create("Monster/RockSlimeEyes.png",
                                                      cocos2d::Rect(x, y, frameW, frameH));
            if (!frame) continue;
            anim->addSpriteFrame(frame);
        }
        // 眼睛动画较慢，每帧 1 秒。
        anim->setDelayPerUnit(1.0f);
        return anim;
    }

    // slimeRunEyesLoop：在史莱姆身体精灵上创建并循环播放眼睛覆盖动画。
    void slimeRunEyesLoop(cocos2d::Sprite* body) const {
        if (!body) return;
        // 通过 Tag 查找是否已经有眼睛子节点。
        auto eyes = dynamic_cast<cocos2d::Sprite*>(body->getChildByTag(kSlimeEyesSpriteTag));
        if (!eyes) {
            auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/RockSlimeEyes.png");
            if (!tex) return;
            tex->setAliasTexParameters();
            auto size = tex->getContentSize();
            if (size.width <= 0.0f || size.height <= 0.0f) return;
            // 与 slimeEyesAnimation 相同：2 行 2 列，每帧 16x24。
            float frameW = size.width / 2.0f;
            float frameH = size.height / 2.0f;
            float texH = size.height;
            eyes = cocos2d::Sprite::create();
            if (!eyes) return;
            eyes->setTexture(tex);
            // 初始显示第 0 帧的眼睛（第 0 行第 0 列）。
            float x0 = 0.0f;
            float y0 = texH - frameH * static_cast<float>(0 + 1);
            eyes->setTextureRect(cocos2d::Rect(x0, y0, frameW, frameH));
            // 眼睛锚点使用中心，方便与身体相对偏移对齐。
            eyes->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
            // 身体小图高度约为 24，这里将眼睛中心略微向右上偏移。
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
