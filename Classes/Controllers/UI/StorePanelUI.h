#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include <functional>
#include "Controllers/Store/StoreController.h"
#include "Game/Item.h"
#include "Game/Animal.h"

namespace Game { class Inventory; }

namespace Controllers {

enum class StoreCategory {
    Produce,
    Mineral
};

class StorePanelUI {
public:
    StorePanelUI(cocos2d::Scene* scene, std::shared_ptr<Game::Inventory> inv)
      : _scene(scene), _inventory(std::move(inv)) {}
    void buildStorePanel();
    void refreshStorePanel();
    void toggleStorePanel(bool show);
    void setCategory(StoreCategory cat);
    StoreCategory category() const { return _category; }
    bool isVisible() const;
    std::function<void(bool)> onPurchased;

private:
    void rebuildItems();
    void updateTitle();
    void updateTabsVisual();

    cocos2d::Scene* _scene = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    cocos2d::Node* _panelNode = nullptr;
    cocos2d::Node* _listNode = nullptr;
    cocos2d::Label* _titleLabel = nullptr;
    cocos2d::Node* _tabsNode = nullptr;
    cocos2d::Label* _produceTab = nullptr;
    cocos2d::Label* _mineralTab = nullptr;
    std::unique_ptr<StoreController> _storeController;
    int _pageIndex = 0;
    int _pageSize = 5;
    StoreCategory _category = StoreCategory::Produce;
    std::vector<Game::ItemType> _items;
};

class AnimalStorePanelUI {
public:
    explicit AnimalStorePanelUI(cocos2d::Scene* scene)
      : _scene(scene) {}
    void buildAnimalStorePanel();
    void refreshAnimalStorePanel();
    void toggleAnimalStorePanel(bool show);
    bool isVisible() const;
    std::function<bool(Game::AnimalType)> onBuyAnimal;
    std::function<void(bool)> onPurchased;

private:
    cocos2d::Scene* _scene = nullptr;
    cocos2d::Node* _panelNode = nullptr;
    cocos2d::Node* _listNode = nullptr;
};

}
