#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <vector>
#include <functional>

namespace Controllers {

class ElevatorPanelUI {
public:
    explicit ElevatorPanelUI(cocos2d::Scene* scene)
      : _scene(scene) {}

    void buildPanel();
    void refreshButtons(const std::vector<int>& floors);
    void togglePanel(bool visible);
    bool isVisible() const;

    void setOnFloorSelected(const std::function<void(int)>& cb) { _onFloorSelected = cb; }

private:
    cocos2d::Scene* _scene = nullptr;
    cocos2d::ui::Layout* _panel = nullptr;
    std::function<void(int)> _onFloorSelected;
};

}

