#pragma once

#include "Game/Tool/Weapon.h"
#include <vector>
#include <utility>

namespace Game {

class Sword : public Weapon {
public:
    ToolKind kind() const override;
    std::string displayName() const override;
    std::string use(Controllers::IMapController* map,
                    Controllers::CropSystem* crop,
                    std::function<cocos2d::Vec2()> getPlayerPos,
                    std::function<cocos2d::Vec2()> getLastDir,
                    Controllers::UIController* ui) override;

    std::string iconPath() const override;

    static int baseDamage();
    static void buildHitTiles(Controllers::IMapController* map,
                              const cocos2d::Vec2& playerPos,
                              const cocos2d::Vec2& lastDir,
                              std::vector<std::pair<int,int>>& outTiles,
                              bool includeSelf);
};

}
