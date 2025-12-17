#pragma once

#include <memory>
#include "cocos2d.h"
#include "Controllers/NPC/NpcControllerBase.h"
#include "Controllers/IMapController.h"
#include "Game/Inventory.h"
#include "Game/NPC/NpcBase.h"

namespace Controllers {

class UIController;

class WillyNpcController : public NpcControllerBase {
 public:
  WillyNpcController(IMapController* map,
                     cocos2d::Node* world_node,
                     UIController* ui,
                     std::shared_ptr<Game::Inventory> inventory,
                     NpcDialogueManager* dialogue);

  void update(const cocos2d::Vec2& player_pos) override;

  void handleTalkAt(const cocos2d::Vec2& player_pos) override;
  bool handleRightClick(cocos2d::EventMouse* e) override;

 private:
  IMapController* map_ = nullptr;
  cocos2d::Node* world_node_ = nullptr;
  UIController* ui_ = nullptr;
  std::shared_ptr<Game::Inventory> inventory_;
  std::unique_ptr<Game::NpcBase> npc_;
  cocos2d::Sprite* sprite_ = nullptr;
  bool was_near_ = false;
  NpcDialogueManager* dialogue_ = nullptr;

  bool isNear(const cocos2d::Vec2& player_pos,
              float max_dist,
              cocos2d::Vec2& out_pos) const;
  int friendshipGainForGift() const;
};

}  // namespace Controllers
