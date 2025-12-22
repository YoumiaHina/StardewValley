#include "Game/Monster/MonsterBase.h"
#include "cocos2d.h"
#include <cmath>

namespace Game {

namespace {

// 根据怪物当前速度向量，决定使用贴图中的哪一行：
// - 速度接近 0 认为静止，固定选第 2 行
// - 横向分量更大时使用左右行（第 1/3 行）
// - 纵向分量更大时使用上下行（第 2/4 行）
int bugRowForVelocity(const Monster& m) {
    cocos2d::Vec2 v = m.velocity;
    float len2 = v.x * v.x + v.y * v.y;
    if (len2 <= 1e-4f) return 4;
    float ax = std::fabs(v.x);
    float ay = std::fabs(v.y);
    if (ax >= ay) {
        return v.x >= 0.0f ? 1 : 3;
    } else {
        return v.y >= 0.0f ? 2 : 4;
    }
}

// 按给定的行号，从 BugMove 贴图中切出该行的 4 帧动画
cocos2d::Animation* bugRowAnimation(int row) {
    auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/BugMove.png");
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
        auto frame = cocos2d::SpriteFrame::create("Monster/BugMove.png", cocos2d::Rect(x, y, frameW, frameH));
        if (!frame) continue;
        anim->addSpriteFrame(frame);
    }
    anim->setDelayPerUnit(0.08f);
    return anim;
}

const int kBugAnimActionTag = 9001;

// 在指定行上循环播放跑步动画：
// - 如果已经在播同一行，则复用现有动作
// - 行号通过 sprite 的 tag 记录，动作通过统一 actionTag 管理
void bugRunLoopRow(int row, cocos2d::Sprite* sprite) {
    if (!sprite) return;
    int currentRow = sprite->getTag();
    auto running = sprite->getActionByTag(kBugAnimActionTag);
    if (running && currentRow == row) return;
    sprite->stopActionByTag(kBugAnimActionTag);
    auto anim = bugRowAnimation(row);
    if (!anim) return;
    sprite->setTag(row);
    auto act = cocos2d::RepeatForever::create(cocos2d::Animate::create(anim));
    act->setTag(kBugAnimActionTag);
    sprite->runAction(act);
}

} // namespace

// Bug 怪物的行为与动画实现：
// - 根据移动方向 / 静止 / 死亡状态，选择 BugMove 贴图中的具体行和帧
// - 通过 MonsterBase 的虚函数接口供怪物系统 / 场景控制器调用
class BugMonster : public MonsterBase {
public:
    // 初始化 Bug 的基础属性与默认掉落配置
    BugMonster() {
        def_.hp = 16;
        def_.dmg = 3;
        def_.def = 0;
        def_.searchRangeTiles = 6;
        def_.moveSpeed = 100.0f;
        def_.isCollisionAffected = false;
        def_.name = "Bug";
        drops_.push_back(ItemType::Fiber);
    }

    // 返回怪物类型枚举，用于系统层区分怪物种类
    Monster::Type monsterType() const override {
        return Monster::Type::Bug;
    }

    // 静止时固定使用第 4 行的循环动画（相当于默认朝向的待机）
    void playStaticAnimation(const Monster& m, cocos2d::Sprite* sprite) const override {
        bugRunLoopRow(4, sprite);
    }

    // 移动时先用 bugRowForVelocity 根据速度方向选行，再播放该行循环动画
    void playMoveAnimation(const Monster& m, cocos2d::Sprite* sprite) const override {
        int row = bugRowForVelocity(m);
        bugRunLoopRow(row, sprite);
    }

    // 死亡时切换到死亡专用行（第 0 行），显示一帧后淡出再通过回调通知系统移除
    void playDeathAnimation(const Monster& m, cocos2d::Sprite* sprite, const std::function<void()>& onComplete) const override {
        if (!sprite) {
            if (onComplete) onComplete();
            return;
        }
        auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/BugMove.png");
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
        float x = 0.0f;
        // 死亡贴图放在第 0 行，这里取第 0 行第 1 列作为死亡帧
        float y = texH - frameH * 1.0f;
        sprite->stopActionByTag(kBugAnimActionTag);
        sprite->setTexture("Monster/BugMove.png");
        sprite->setTextureRect(cocos2d::Rect(x, y, frameW, frameH));
        sprite->setOpacity(255);
        auto delay = cocos2d::DelayTime::create(0.2f);
        auto fade = cocos2d::FadeOut::create(0.25f);
        auto seq = cocos2d::Sequence::create(
            delay,
            fade,
            cocos2d::CallFunc::create([onComplete]() {
                if (onComplete) onComplete();
            }),
            nullptr);
        sprite->runAction(seq);
    }
};

const MonsterBase& bugMonsterBehavior() {
    static BugMonster inst;
    return inst;
}

} // namespace Game
