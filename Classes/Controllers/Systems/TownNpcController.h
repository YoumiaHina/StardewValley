#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include "Game/Npc.h"
#include "Game/Inventory.h"

namespace Controllers {

class TownMapController;
class UIController;

class TownNpcController {
public:
    TownNpcController(TownMapController* map,
                      cocos2d::Node* worldNode,
                      UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory);

    void setInventory(std::shared_ptr<Game::Inventory> inventory) { _inventory = std::move(inventory); }

    void update(const cocos2d::Vec2& playerPos);

    bool findNearestNpc(const cocos2d::Vec2& playerPos,
                        float maxDist,
                        Game::NpcId& outId,
                        cocos2d::Vec2& outPos) const;

    void handleTalkAt(const cocos2d::Vec2& playerPos);

private:
    struct NpcInstance {
        Game::NpcId id;
        cocos2d::Sprite* sprite = nullptr;
    };

    TownMapController* _map = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    UIController* _ui = nullptr;
    std::shared_ptr<Game::Inventory> _inventory;
    std::vector<NpcInstance> _npcs;

    void buildInitialNpcs();
};

} // namespace Controllers
