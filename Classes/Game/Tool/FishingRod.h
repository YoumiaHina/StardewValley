#pragma once

#include "Game/Tool/ToolBase.h"
#include <functional>

namespace Game {

class FishingRod : public ToolBase {
public:
    ToolKind kind() const override;
    std::string displayName() const override;
    std::string use(Controllers::IMapController* map,
                    Controllers::CropSystem* crop,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir,
                    Controllers::UIController* ui) override;
    void setFishingStarter(std::function<void(const cocos2d::Vec2&)> cb);

    std::string iconPath() const override;
private:
    std::function<void(const cocos2d::Vec2&)> _onStartFishing;
};

}
