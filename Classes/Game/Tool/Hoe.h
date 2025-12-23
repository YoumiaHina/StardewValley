#pragma once

#include "Game/Tool/ToolBase.h"

namespace Game {

// 锄头工具：用于翻地、收获部分作物。
// 继承自 ToolBase，只承担与“锄地/收获”相关的行为。
class Hoe : public ToolBase {
public:
    // 返回工具类型枚举 ToolKind::Hoe。
    ToolKind kind() const override;
    // 返回在 UI 中显示的名称（"Hoe"）。
    std::string displayName() const override;
    // 使用锄头：
    // - 检查体力是否足够；
    // - 在农场地图上，优先尝试收获成熟作物；
    // - 若格子是普通土壤且没有树/石头/杂草阻挡，则翻地变成耕地或浇水后的耕地；
    // - 刷新地图/作物可视和 HUD。
    std::string use(Controllers::IMapController* map,
                    Controllers::CropSystem* crop,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir,
                    Controllers::UIController* ui) override;

    // 根据锄头等级返回不同的图标路径。
    std::string iconPath() const override;
};

}
