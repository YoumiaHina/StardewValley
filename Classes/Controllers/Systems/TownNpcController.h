#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include "Game/Inventory.h"
#include "Game/Npc.h"

namespace Controllers {

class TownMapController;
class UIController;

class NpcControllerBase {
 public:
  virtual ~NpcControllerBase() = default;
  virtual void update(const cocos2d::Vec2& player_pos) = 0;
  virtual void handleTalkAt(const cocos2d::Vec2& player_pos) = 0;
};

class TownNpcController : public NpcControllerBase {
 public:
  TownNpcController(TownMapController* map,
                    cocos2d::Node* world_node,
                    UIController* ui,
                    std::shared_ptr<Game::Inventory> inventory);

  void setInventory(std::shared_ptr<Game::Inventory> inventory) {
    inventory_ = std::move(inventory);
  }

  void update(const cocos2d::Vec2& player_pos) override;

  void handleTalkAt(const cocos2d::Vec2& player_pos) override;

 private:
  struct NpcInstance {
    Game::NpcId id;
    std::unique_ptr<Game::NpcBase> npc;
    cocos2d::Sprite* sprite = nullptr;
  };

  TownMapController* map_ = nullptr;
  cocos2d::Node* world_node_ = nullptr;
  UIController* ui_ = nullptr;
  std::shared_ptr<Game::Inventory> inventory_;
  std::vector<NpcInstance> npcs_;

  void buildInitialNpcs();
  bool findNearestNpc(const cocos2d::Vec2& player_pos,
                      float max_dist,
                      int& out_index,
                      cocos2d::Vec2& out_pos) const;
  int friendshipGainForGift(NpcInstance& inst) const;
};

} // namespace Controllers

