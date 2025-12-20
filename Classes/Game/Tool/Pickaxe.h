#pragma once

#include "Game/Tool/ToolBase.h"

namespace Game {

class Pickaxe : public ToolBase {
public:
    ToolKind kind() const override;
    std::string displayName() const override;
    std::string use(Controllers::IMapController* map,
                    Controllers::CropSystem* crop,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir,
                    Controllers::UIController* ui) override;

    std::string iconPath() const override;
};

}
