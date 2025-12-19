#pragma once

#include "cocos2d.h"

namespace Controllers {

class HUDUI {
public:
    HUDUI(cocos2d::Scene* scene) : _scene(scene) {}
    void buildHUD();
    void refreshHUD();
    void buildHPBarAboveEnergy();
    void buildMineFloorLabel();
    void setMineFloorNumber(int floor);

private:
    cocos2d::Scene* _scene = nullptr;
    cocos2d::Label* _hudTimeLabel = nullptr;
    cocos2d::Label* _hudWeatherLabel = nullptr;
    cocos2d::Label* _hudGoldLabel = nullptr;
    cocos2d::Node* _energyNode = nullptr;
    cocos2d::DrawNode* _energyFill = nullptr;
    cocos2d::Label* _energyLabel = nullptr;
    cocos2d::Node* _hpNode = nullptr;
    cocos2d::DrawNode* _hpFill = nullptr;
    cocos2d::Label* _hpLabel = nullptr;
    cocos2d::Label* _mineFloorLabel = nullptr;
};

}
