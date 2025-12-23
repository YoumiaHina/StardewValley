#pragma once

#include "Game/Tool/ToolBase.h"

namespace Game {

// 镰刀工具：用于割草类障碍（Weed），并产生对应掉落。
// 继承自 ToolBase，但不依赖 CropSystem。
class Scythe : public ToolBase {
public:
    // 返回工具类型枚举 ToolKind::Scythe。
    ToolKind kind() const override;
    // 返回在 UI 中显示的名称（"Scythe"）。
    std::string displayName() const override;
    // 使用镰刀：
    // - 检查体力；
    // - 计算玩家面前的目标格子；
    // - 调用 Weed 障碍系统的 damageAt 进行破坏并生成掉落；
    // - 刷新 HUD 与地图。
    std::string use(Controllers::IMapController* map,
                    Controllers::CropSystem* crop,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir,
                    Controllers::UIController* ui) override;

    // 镰刀图标路径固定，不随等级变化。
    std::string iconPath() const override;
};

}
