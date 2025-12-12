#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include <functional>
#include "Controllers/Store/StoreController.h"
#include "Game/Item.h"

namespace Game { class Inventory; }

namespace Controllers {

class StorePanelUI {
public:
    StorePanelUI(cocos2d::Scene* scene, std::shared_ptr<Game::Inventory> inv)
      : _scene(scene), _inventory(std::move(inv)) {}
    void buildStorePanel();
    void refreshStorePanel();
    void toggleStorePanel(bool show);
    std::function<void(bool)> onPurchased;

private:
    cocos2d::Scene* _scene = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    cocos2d::Node* _panelNode = nullptr;
    cocos2d::Node* _listNode = nullptr;
    std::unique_ptr<StoreController> _storeController;
    int _pageIndex = 0;
    int _pageSize = 6;
    std::vector<Game::ItemType> _items;
};

}
