#pragma once

#include "cocos2d.h"
#include <memory>

namespace Game {
class Inventory;
}

namespace Controllers {

class IMapController;
class UIController;

class PlaceableItemSystemBase {
public:
    PlaceableItemSystemBase()
    : _parentNode(nullptr)
    , _drawNode(nullptr) {}

    virtual ~PlaceableItemSystemBase() = default;

    void attachTo(cocos2d::Node* parentNode, int zOrder) {
        _parentNode = parentNode;
        if (!_parentNode) return;
        _drawNode = cocos2d::DrawNode::create();
        _parentNode->addChild(_drawNode, zOrder);
    }

    bool InteractWithItem(Controllers::IMapController* map,
                          Controllers::UIController* ui,
                          const std::shared_ptr<Game::Inventory>& inventory,
                          const cocos2d::Vec2& playerWorldPos,
                          const cocos2d::Vec2& lastDir) {
        if (!map || !ui) return false;
        if (inventory && shouldPlace(*inventory)) {
            bool placed = tryPlace(map, ui, inventory, playerWorldPos, lastDir);
            if (placed) return true;
        }
        return tryInteractExisting(map, ui, inventory, playerWorldPos, lastDir);
    }

protected:
    virtual bool shouldPlace(const Game::Inventory& inventory) const = 0;

    virtual bool tryPlace(Controllers::IMapController* map,
                          Controllers::UIController* ui,
                          const std::shared_ptr<Game::Inventory>& inventory,
                          const cocos2d::Vec2& playerWorldPos,
                          const cocos2d::Vec2& lastDir) = 0;

    virtual bool tryInteractExisting(Controllers::IMapController* map,
                                     Controllers::UIController* ui,
                                     const std::shared_ptr<Game::Inventory>& inventory,
                                     const cocos2d::Vec2& playerWorldPos,
                                     const cocos2d::Vec2& lastDir) = 0;

    cocos2d::Node* _parentNode;
    cocos2d::DrawNode* _drawNode;
};

}

