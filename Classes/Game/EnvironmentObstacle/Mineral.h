#pragma once

#include "cocos2d.h"
#include <string>
#include <functional>
#include "Game/Item.h"
#include "Game/EnvironmentObstacle/EnvironmentObstacleBase.h"

namespace Game {

// 矿石/矿物类型：用于决定贴图、耐久与掉落物。
enum class MineralType {
    Rock,
    HardRock,
    HugeRock,
    CopperOre,
    IronOre,
    GoldOre
};

// 矿石障碍物节点：
// - 作用：在地图上展示并承载单个矿石实体的受击、HP 与销毁动画。
// - 职责边界：只管理单体矿石的可视化与局部状态；生成/存储/刷新由对应系统负责。
// - 主要协作对象：矿石系统驱动其受击与销毁，并根据 `mineralDropItem` 生成掉落。
class Mineral : public EnvironmentObstacleBase {
public:
    // 工厂创建：使用指定贴图路径创建矿石实例。
    static Mineral* create(const std::string& texture);
    // 初始化：加载贴图并构建内部 Sprite。
    bool initWithTexture(const std::string& texture);
    // 设置破碎态贴图（用于受击/销毁时切换贴图）。
    void setBrokenTexture(const std::string& texture);
    // 设置矿石类型（用于掉落/耐久等外部逻辑识别）。
    void setType(MineralType type) { _type = type; }
    // 获取矿石类型。
    MineralType type() const { return _type; }

    // 施加伤害：降低 HP，并在死亡时进入破坏流程。
    void applyDamage(int amount) override;
    // 获取当前生命值。
    int hp() const override { return _hp; }
    // 是否已死亡。
    bool dead() const override { return _hp <= 0; }

    // 获取脚底碰撞矩形（用于近地面阻挡与命中判定）。
    cocos2d::Rect footRect() const override;
    // 获取当前精灵的内容尺寸（用于外部布局/碰撞计算）。
    cocos2d::Size spriteContentSize() const;
    // 播放销毁动画并在完成后回调。
    void playDestructionAnimation(const std::function<void()>& onComplete) override;

private:
    cocos2d::Sprite* _sprite = nullptr;
    int _hp = 1;
    bool _breaking = false;
    std::string _brokenTexture;
    MineralType _type = MineralType::Rock;
};

// 矿石运行时数据：用于系统层保存与重建矿石状态。
struct MineralData {
    MineralType type;
    cocos2d::Vec2 pos;
    int hp = 1;
    int sizeTiles = 1;
    std::string texture;
};

// 根据矿石类型返回对应的掉落物品类型。
ItemType mineralDropItem(MineralType type);

}

