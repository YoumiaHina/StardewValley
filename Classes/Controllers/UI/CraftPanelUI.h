#pragma once

#include "cocos2d.h"

namespace Controllers {

class CraftPanelUI {
public:
    CraftPanelUI(cocos2d::Scene* scene) : _scene(scene) {}
    void buildCraftPanel();
    void refreshCraftPanel(int level);
    void toggleCraftPanel(bool show);

private:
    cocos2d::Scene* _scene = nullptr;
    cocos2d::Node* _panelNode = nullptr;
};

}
