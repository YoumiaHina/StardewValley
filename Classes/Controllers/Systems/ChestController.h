#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include "Controllers/Systems/PlaceableItemSystemBase.h"
#include "Game/Chest.h"
#include "Game/Inventory.h"

namespace Controllers {

class IMapController;
class UIController;
class RoomMapController;

class ChestController : public PlaceableItemSystemBase {
public:
    explicit ChestController(bool isFarm)
    : PlaceableItemSystemBase()
    , _isFarm(isFarm) {}

    void attachTo(cocos2d::Node* parentNode, int zOrder);
    void syncLoad();

    const std::vector<Game::Chest>& chests() const;
    std::vector<Game::Chest>& chests();

    bool isNearChest(const cocos2d::Vec2& worldPos) const;
    bool collides(const cocos2d::Vec2& worldPos) const;

    void refreshVisuals();

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

    bool _isFarm = true;
    std::vector<Game::Chest> _chests;
};

bool openChestNearPlayer(Controllers::IMapController* map,
                         Controllers::UIController* ui,
                         const cocos2d::Vec2& playerWorldPos,
                         const cocos2d::Vec2& lastDir);

bool openGlobalChest(Controllers::UIController* ui);

bool placeChestOnFarm(Controllers::IMapController* map,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const cocos2d::Vec2& playerPos,
                      const cocos2d::Vec2& lastDir);

bool placeChestInRoom(Controllers::RoomMapController* room,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const cocos2d::Vec2& playerPos);

bool placeChestOnOutdoorMap(Controllers::IMapController* map,
                            Controllers::UIController* ui,
                            std::shared_ptr<Game::Inventory> inventory,
                            const cocos2d::Vec2& playerPos);

void transferChestCell(Game::Chest& chest,
                       int flatIndex,
                       Game::Inventory& inventory);

}
