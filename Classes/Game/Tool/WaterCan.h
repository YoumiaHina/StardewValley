#pragma once

#include "Game/Tool/ToolBase.h"

namespace Game {

class WaterCan : public ToolBase {
public:
    ~WaterCan() override;
    ToolKind kind() const override;
    std::string displayName() const override;
    std::string use(Controllers::IMapController* map,
                    Controllers::CropSystem* crop,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir,
                    Controllers::UIController* ui) override;

    std::string iconPath() const override;

    void attachHotbarOverlay(cocos2d::Sprite* iconSprite, float cellW, float cellH) override;
    void refreshHotbarOverlay() override;
    void detachHotbarOverlay() override;

private:
    cocos2d::Node* _waterBarNode = nullptr;
    cocos2d::DrawNode* _waterBarBg = nullptr;
    cocos2d::DrawNode* _waterBarFill = nullptr;
    cocos2d::Sprite* _iconSprite = nullptr;
    float _cellW = 0.f;
    float _cellH = 0.f;
};

}
