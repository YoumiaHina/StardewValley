#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include <string>
#include "Game/Inventory.h"

namespace Controllers {

class HotbarUI {
public:
    HotbarUI(cocos2d::Scene* scene,
             std::shared_ptr<Game::Inventory> inventory)
    : _scene(scene), _inventory(std::move(inventory)) {}

    void setInventoryBackground(const std::string& path);
    void buildHotbar();
    void refreshHotbar();
    void selectHotbarIndex(int idx);
    bool handleHotbarMouseDown(cocos2d::EventMouse* e);
    bool handleHotbarAtPoint(const cocos2d::Vec2& screenPoint);
    void handleHotbarScroll(float dy);

    float getScale() const { return _hotbarScale; }
    void setScale(float s) { _hotbarScale = s; }

private:
    cocos2d::Scene* _scene = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;

    cocos2d::Node* _hotbarNode = nullptr;
    cocos2d::DrawNode* _hotbarHighlight = nullptr;
    std::vector<cocos2d::Label*> _hotbarLabels;
    std::vector<cocos2d::Sprite*> _hotbarIcons;
    cocos2d::Label* _selectedHintLabel = nullptr;
    cocos2d::Sprite* _hotbarBgSprite = nullptr;
    std::string _inventoryBgPath;
    float _hotbarScale = 1.0f;
};

}
