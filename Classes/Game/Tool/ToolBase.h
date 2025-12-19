#pragma once

#include <string>
#include <functional>
#include "cocos2d.h"

namespace Controllers { class IMapController; class UIController; class CropSystem; }

namespace Game {

enum class ToolKind { Axe, Hoe, Pickaxe, WaterCan, FishingRod, Sword, Scythe };

class ToolBase {
public:
    virtual ~ToolBase() = default;
    virtual ToolKind kind() const = 0;
    virtual std::string displayName() const = 0;
    virtual std::string use(Controllers::IMapController* map,
                            Controllers::CropSystem* crop,
                            std::function<cocos2d::Vec2()> getPlayerPos,
                            std::function<cocos2d::Vec2()> getLastDir,
                            Controllers::UIController* ui) = 0;

    virtual void attachHotbarOverlay(cocos2d::Sprite* /*iconSprite*/, float /*cellW*/, float /*cellH*/) {}
    virtual void refreshHotbarOverlay() {}
    virtual void detachHotbarOverlay() {}
};

}
