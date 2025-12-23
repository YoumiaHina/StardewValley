#pragma once

#include "Game/Tool/ToolBase.h"

namespace Game {

// 浇水壶工具：用于给耕地浇水，并且在靠近水面时可以为自身“补水”。
// 除了常规的 use 逻辑，还负责在热键栏图标上显示当前水量条。
class WaterCan : public ToolBase {
public:
    // 析构函数需要清理热键栏上的水量条节点，因此在 .cpp 中实现。
    ~WaterCan() override;
    // 返回工具类型枚举 ToolKind::WaterCan。
    ToolKind kind() const override;
    // 返回在 UI 中显示的名称（"Water Can"）。
    std::string displayName() const override;
    // 使用浇水壶：
    // - 若玩家靠近水面且水量未满，则为浇水壶补满水，不消耗体力；
    // - 否则，检测体力并对前方格子浇水（只对已耕地有效）；
    // - 调整 WorldState 中的 water/maxWater，并刷新 HUD/热键栏。
    std::string use(Controllers::IMapController* map,
                    Controllers::CropSystem* crop,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir,
                    Controllers::UIController* ui) override;

    // 根据等级返回不同前缀的图标路径（基础/铜/铁/金）。
    std::string iconPath() const override;

    // 在背包热键栏的某个格子上挂载水量条：
    // - iconSprite：该格子的图标精灵；
    // - cellW/cellH：格子宽高，用于计算水量条的大小与位置。
    void attachHotbarOverlay(cocos2d::Sprite* iconSprite, float cellW, float cellH) override;
    // 当水量或最大水量发生变化时，重绘水量条填充部分。
    void refreshHotbarOverlay() override;
    // 把水量条节点从父节点移除，避免内存泄漏或悬挂指针。
    void detachHotbarOverlay() override;

private:
    // 下列指针指向的是 Cocos 节点，由 attachHotbarOverlay 创建并挂载到 UI 上。
    cocos2d::Node* _waterBarNode = nullptr;
    cocos2d::DrawNode* _waterBarBg = nullptr;
    cocos2d::DrawNode* _waterBarFill = nullptr;
    cocos2d::Sprite* _iconSprite = nullptr;
    float _cellW = 0.f;
    float _cellH = 0.f;
};

}
