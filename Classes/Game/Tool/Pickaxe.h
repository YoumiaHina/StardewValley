#pragma once

#include "Game/Tool/ToolBase.h"

namespace Game {

// 镐子工具：用于敲碎矿石、平整农田。
// 继承自 ToolBase，只关注与“挖矿/整地”相关的行为。
class Pickaxe : public ToolBase {
public:
    // 返回工具类型枚举 ToolKind::Pickaxe。
    ToolKind kind() const override;
    // 返回在 UI 中显示的名称（"Pickaxe"）。
    std::string displayName() const override;
    // 使用镐子：
    // - 检查体力是否足够；
    // - 根据玩家位置和朝向，选择单格或扇形区域的目标格子；
    // - 优先尝试破坏矿石并生成掉落；
    // - 在农场地图上还能把耕地/浇水后的耕地拍平为普通土壤。
    std::string use(Controllers::IMapController* map,
                    Controllers::CropSystem* crop,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir,
                    Controllers::UIController* ui) override;

    // 根据等级返回不同前缀的图标路径。
    std::string iconPath() const override;
};

}
