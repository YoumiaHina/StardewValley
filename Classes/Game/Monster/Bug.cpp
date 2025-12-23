#include "Game/Monster/MonsterBase.h"
#include "cocos2d.h"
#include <cmath>

namespace Game {

// 匿名 namespace：只在本 cpp 文件内部可见的“私有”函数和常量。
// - 等价于 C 里的 static 函数，这里用匿名命名空间来限制作用域。
namespace {

// 根据速度向量选择 BugMove.png 中的哪一行：
// - v 是 cocos2d::Vec2，包含 x、y 两个 float 分量；
// - 先看速度是否接近 0（静止），再比较 |x| 和 |y|，判断主要朝哪个方向移动。
int bugRowForVelocity(const cocos2d::Vec2& v) {
    float len2 = v.x * v.x + v.y * v.y;
    if (len2 <= 1e-4f) return 4; // 几乎不动：使用第 4 行（“待机”行）
    float ax = std::fabs(v.x);
    float ay = std::fabs(v.y);
    if (ax >= ay) {
        // 水平方向为主：向右用第 1 行，向左用第 3 行。
        return v.x >= 0.0f ? 1 : 3;
    } else {
        // 垂直方向为主：向上用第 2 行，向下用第 4 行。
        return v.y >= 0.0f ? 2 : 4;
    }
}

// bugRowAnimation：按给定的行号，从 BugMove 贴图中切出该行的 4 帧动画。
cocos2d::Animation* bugRowAnimation(int row) {
    auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage("Monster/BugMove.png");
    if (!tex) return nullptr;
    tex->setAliasTexParameters();
    auto size = tex->getContentSize();
    if (size.width <= 0.0f || size.height <= 0.0f) return nullptr;
    float frameW = size.width / 4.0f; // 每行 4 列。
    float frameH = size.height / 5.0f; // 共 5 行。
    float texH = size.height;
    auto anim = cocos2d::Animation::create();
    if (!anim) return nullptr;
    for (int col = 0; col < 4; ++col) {
        float x = frameW * static_cast<float>(col);
        // Cocos2d 纹理原点在左下，这里用 texH - frameH*(row+1) 计算 y。
        float y = texH - frameH * static_cast<float>(row + 1);
        auto frame = cocos2d::SpriteFrame::create("Monster/BugMove.png", cocos2d::Rect(x, y, frameW, frameH));
        if (!frame) continue;
        anim->addSpriteFrame(frame);
    }
    anim->setDelayPerUnit(0.08f);
    return anim;
}

// 用来标记“Bug 动画动作”的 Tag，方便后续 stopActionByTag。
const int kBugAnimActionTag = 9001;

// bugRunLoopRow：在指定行上循环播放跑步动画。
// - 如果已经在播同一行，则直接复用现有动作，不重新创建；
// - 精灵的 tag 字段被用来记录“当前行号”。
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

// BugMonster：Bug 怪物的行为与动画实现。
// - 根据移动方向 / 静止 / 死亡状态，选择 BugMove 贴图中的具体行和帧；
// - 通过 MonsterBase 的虚函数接口供怪物系统 / 场景控制器调用。
class BugMonster : public MonsterBase {
public:
    // 构造函数：初始化 Bug 的基础属性与默认掉落配置。
    BugMonster() {
        def_.hp = 16;
        def_.dmg = 3;
        def_.def = 0;
        def_.searchRangeTiles = 6;
        def_.moveSpeed = 100.0f;
        def_.isCollisionAffected = false; // 不受碰撞影响，可以直接穿过障碍。
        def_.name = "Bug";
        drops_.push_back(ItemType::Fiber);
    }

    MonsterType monsterType() const override {
        return MonsterType::Bug;
    }

    // 静止动画：使用第 4 行（“待机/向下”）的动画。
    void playStaticAnimation(cocos2d::Sprite* sprite) const override {
        bugRunLoopRow(4, sprite);
    }

    // 移动动画：根据速度计算行号，然后播放对应行。
    void playMoveAnimation(const cocos2d::Vec2& velocity, cocos2d::Sprite* sprite) const override {
        int row = bugRowForVelocity(velocity);
        bugRunLoopRow(row, sprite);
    }

    // 死亡动画：选择 BugMove 第 0 行作为死亡帧，做一个淡出的效果。
    void playDeathAnimation(cocos2d::Sprite* sprite, const std::function<void()>& onComplete) const override {
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
        // 死亡贴图放在第 0 行，这里取第 0 行第 1 列作为死亡帧。
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

// bugMonsterBehavior：返回静态行为对象引用。
const MonsterBase& bugMonsterBehavior() {
    static BugMonster inst;
    return inst;
}

} // namespace Game
