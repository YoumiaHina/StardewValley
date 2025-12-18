#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include "Controllers/Systems/PlaceableItemSystemBase.h"
#include "Game/Furnace.h"
#include "Game/Inventory.h"

namespace Controllers {

class IMapController;
class UIController;

class FurnaceController : public PlaceableItemSystemBase {
public:
    FurnaceController();

    void attachTo(cocos2d::Node* parentNode, int zOrder);

    void bindContext(Controllers::IMapController* map,
                     Controllers::UIController* ui,
                     std::shared_ptr<Game::Inventory> inventory);

    void syncLoad();
    void update(float dt);
    void refreshVisuals();

    const std::vector<Game::Furnace>& furnaces() const;
    std::vector<Game::Furnace>& furnaces();

    bool interactAt(const cocos2d::Vec2& playerWorldPos,
                    const cocos2d::Vec2& lastDir);
    bool isNearFurnace(const cocos2d::Vec2& worldPos) const;
    bool collides(const cocos2d::Vec2& worldPos) const;

private:
    bool shouldPlace(const Game::Inventory& inventory) const override;

    bool tryPlace(Controllers::IMapController* map,
                  Controllers::UIController* ui,
                  const std::shared_ptr<Game::Inventory>& inventory,
                  const cocos2d::Vec2& playerWorldPos,
                  const cocos2d::Vec2& lastDir) override;

    bool tryInteractExisting(Controllers::IMapController* map,
                             Controllers::UIController* ui,
                             const std::shared_ptr<Game::Inventory>& inventory,
                             const cocos2d::Vec2& playerWorldPos,
                             const cocos2d::Vec2& lastDir) override;

    int findNearestFurnace(const cocos2d::Vec2& playerWorldPos, float maxDist) const;

    Controllers::IMapController* _map = nullptr;
    Controllers::UIController* _ui = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    std::vector<Game::Furnace>* _runtime = nullptr;
};

}
