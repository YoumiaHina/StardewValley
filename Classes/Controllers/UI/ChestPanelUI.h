#pragma once

#include "cocos2d.h"
#include <memory>
#include "Game/Inventory.h"
#include "Game/Chest.h"

namespace Controllers {

class ChestPanelUI {
public:
    ChestPanelUI(cocos2d::Scene* scene, std::shared_ptr<Game::Inventory> inv)
      : _scene(scene), _inventory(std::move(inv)) {}
    void buildChestPanel();
    void refreshChestPanel(Game::Chest* chest);
    void toggleChestPanel(bool show);
    bool isVisible() const;

private:
    cocos2d::Scene* _scene = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    cocos2d::Node* _panelNode = nullptr;
    Game::Chest* _currentChest = nullptr;
    cocos2d::Node* _slotsRoot = nullptr;
    cocos2d::DrawNode* _highlightNode = nullptr;
};

}
