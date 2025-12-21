#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include <functional>
#include "Game/Inventory.h"

namespace Controllers {

class ToolUpgradePanelUI {
public:
    ToolUpgradePanelUI(cocos2d::Scene* scene,
                       std::shared_ptr<Game::Inventory> inv)
      : _scene(scene), _inventory(std::move(inv)) {}

    void buildPanel();
    void refreshPanel();
    void togglePanel(bool show);
    bool isVisible() const;

    void setOnUpgraded(const std::function<void()>& cb);

private:
    struct RowWidgets {
        cocos2d::Sprite* toolIcon = nullptr;
        cocos2d::Label* levelLabel = nullptr;
        cocos2d::Label* buttonLabel = nullptr;
        Game::ToolKind kind = Game::ToolKind::Axe;
        bool canUpgrade = false;
        std::vector<cocos2d::Sprite*> materialIcons;
    };

    cocos2d::Scene* _scene = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    cocos2d::Node* _panelNode = nullptr;
    std::vector<RowWidgets> _rows;
    std::function<void()> _onUpgraded;
};

}
