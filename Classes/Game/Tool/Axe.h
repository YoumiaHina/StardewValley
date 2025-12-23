#pragma once

#include "Game/Tool/ToolBase.h"

namespace Game {

// 斧头工具：用于砍树、破坏树类障碍物。
// 继承自 ToolBase，只关心“斧头相关”的行为与图标路径。
class Axe : public ToolBase {
public:
    // 返回工具类型枚举 ToolKind::Axe。
    ToolKind kind() const override;
    // 返回在 UI 中显示的名称（"Axe"）。
    std::string displayName() const override;
    // 使用斧头：
    // - 检查体力是否足够；
    // - 根据玩家位置和朝向，计算要作用的格子（单格或扇形范围）；
    // - 调用环境系统中“树”的 System 执行伤害与掉落；
    // - 刷新地图与 HUD，并返回提示文字（"Chop!" 或 "Nothing"）。
    std::string use(Controllers::IMapController* map,
                    Controllers::CropSystem* crop,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir,
                    Controllers::UIController* ui) override;

    // 根据斧头等级返回不同的图标路径（基础/铜/铁/金）。
    std::string iconPath() const override;
};

}
