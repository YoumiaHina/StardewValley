#pragma once

#include <memory>
#include "cocos2d.h"
#include "Controllers/NPC/NpcControllerBase.h"
#include "Controllers/Map/IMapController.h"
#include "Game/Inventory.h"
#include "Game/NPC/NpcBase.h"

namespace Controllers {

class UIController;

class AbigailNpcController : public NpcControllerBase {
 public:
  AbigailNpcController(IMapController* map,
                       cocos2d::Node* world_node,
                       UIController* ui,
                       std::shared_ptr<Game::Inventory> inventory,
                       NpcDialogueManager* dialogue);
  ~AbigailNpcController() override;

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

  float tile_ = 0.0f;
  cocos2d::Vec2 patrol_base_pos_;
  cocos2d::Animation* walk_down_ = nullptr;
  cocos2d::Animation* walk_up_ = nullptr;
  cocos2d::Animation* walk_left_ = nullptr;
  cocos2d::Animation* walk_right_ = nullptr;
  int facing_dir_ = 0;

  bool isNear(const cocos2d::Vec2& player_pos,
              float max_dist,
              cocos2d::Vec2& out_pos) const;
  int friendshipGainForGift() const;

  void startPatrol();
  void playWalk(int dir);
  void setStanding(int dir);
};

}  // namespace Controllers
