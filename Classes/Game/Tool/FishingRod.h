#pragma once

#include "Game/Tool/ToolBase.h"
#include <functional>

namespace Game {

// 钓鱼竿工具：用于在水边开始一次钓鱼交互。
// 真正的钓鱼小游戏由外部系统负责，这里只负责触发入口。
class FishingRod : public ToolBase {
public:
    // 返回工具类型枚举 ToolKind::FishingRod。
    ToolKind kind() const override;
    // 返回在 UI 中显示的名称（"Fishing Rod"）。
    std::string displayName() const override;
    // 使用钓鱼竿：
    // - 检查玩家是否站在水边；
    // - 若在水边，则调用 _onStartFishing 回调启动钓鱼流程，并返回 "Fishing..."；
    // - 否则提示 "Need water"。
    std::string use(Controllers::IMapController* map,
                    Controllers::CropSystem* crop,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir,
                    Controllers::UIController* ui) override;
    // 设置一个回调函数，在成功开始钓鱼时被调用。
    // 使用 std::function<void(const cocos2d::Vec2&)> 可以灵活保存 lambda 或普通函数。
    void setFishingStarter(std::function<void(const cocos2d::Vec2&)> cb);

    // 返回图标路径（当前固定为 "Tool/FishingRod.png"）。
    std::string iconPath() const override;
private:
    // 当玩家在水边使用该工具时触发的回调，可由外部系统设置。
    std::function<void(const cocos2d::Vec2&)> _onStartFishing;
};

}
