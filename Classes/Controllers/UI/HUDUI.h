#pragma once

#include "cocos2d.h"

namespace Controllers {

// HUDUI：
// - 作用：构建并刷新 HUD（时间/天气/金币/体力），以及矿洞专属的血条与楼层显示。
// - 职责边界：只负责 HUD 节点的创建与展示刷新，不负责计算时间/天气/金币/HP 等业务数据。
// - 主要协作对象：由 UIController 持有并在合适时机调用 build/refresh；数据由系统层通过全局状态或控制器读取。
class HUDUI {
public:
    // 构造：记录所属场景，用于挂接 HUD 节点。
    HUDUI(cocos2d::Scene* scene) : _scene(scene) {}
    // 构建 HUD 节点（只创建一次，后续复用）。
    void buildHUD();
    // 刷新 HUD 显示内容（时间/天气/金币/能量等）。
    void refreshHUD();
    // 矿洞专属：在能量条上方构建血条。
    void buildHPBarAboveEnergy();
    // 矿洞专属：构建左上角楼层标签。
    void buildMineFloorLabel();
    // 矿洞专属：刷新楼层标签显示的楼层数字。
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
