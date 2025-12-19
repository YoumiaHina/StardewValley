#pragma once

#include <memory>
#include "cocos2d.h"
#include "Controllers/NPC/NpcControllerBase.h"
#include "Game/Inventory.h"
#include "Game/NPC/NpcBase.h"

namespace Controllers {

class FarmMapController;
class UIController;
class AnimalSystem;

class RobinNpcController : public NpcControllerBase {
 public:
  RobinNpcController(FarmMapController* map,
                     cocos2d::Node* world_node,
                     UIController* ui,
                     std::shared_ptr<Game::Inventory> inventory,
                     AnimalSystem* animals);
  ~RobinNpcController() override;

  void update(const cocos2d::Vec2& player_pos) override;

  void handleTalkAt(const cocos2d::Vec2& player_pos) override;
  bool handleRightClick(cocos2d::EventMouse* e) override;

 private:
  FarmMapController* map_ = nullptr;
  cocos2d::Node* world_node_ = nullptr;
  UIController* ui_ = nullptr;
  std::shared_ptr<Game::Inventory> inventory_;
  std::unique_ptr<Game::NpcBase> npc_;
  cocos2d::Sprite* sprite_ = nullptr;
  bool was_near_ = false;
  AnimalSystem* animals_ = nullptr;

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

  void startPatrol();
  void playWalk(int dir);
  void setStanding(int dir);
};

}  // namespace Controllers

