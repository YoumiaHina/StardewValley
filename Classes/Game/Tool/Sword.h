#pragma once

#include "Game/Tool/Weapon.h"
#include <vector>
#include <utility>

namespace Game {

// 剑（近战武器）：
// - 继承自 Weapon，后续可以在 Weapon 中加入通用武器属性（伤害、范围等）；
// - 当前主要用来展示挥砍动作和计算攻击范围。
class Sword : public Weapon {
public:
    // 返回工具类型枚举 ToolKind::Sword。
    ToolKind kind() const override;
    // 返回在 UI 中显示的名称（"Sword"）。
    std::string displayName() const override;
    // 使用剑：
    // - 检查体力并扣除；
    // - 弹出 "Slash!" 提示，并刷新 HUD/热键栏；
    // - 地图可视会轻微刷新，以保持与其他工具行为一致。
    // 真实的战斗判定在其它系统中（例如怪物控制器等）。
    std::string use(Controllers::IMapController* map,
                    Controllers::CropSystem* crop,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir,
                    Controllers::UIController* ui) override;

    // 根据当前深度（矿井电梯最高层数）决定显示哪一把剑的图标。
    std::string iconPath() const override;

    // 基础伤害：根据玩家解锁的电梯层数增加额外加成。
    int baseDamage() const override;
    // 攻击冷却时间：两次挥剑之间的最小间隔（秒），限制最高攻击频率。
    float attackCooldownSeconds() const override;
    // 计算挥剑时可能命中的地图格子集合：
    // - 使用 TileSelector 的“前方扇形区域”算法；
    // - includeSelf 为 true 时，会包含玩家所在格子。
    static void buildHitTiles(Controllers::IMapController* map,
                              const cocos2d::Vec2& playerPos,
                              const cocos2d::Vec2& lastDir,
                              std::vector<std::pair<int,int>>& outTiles,
                              bool includeSelf);
};

}
