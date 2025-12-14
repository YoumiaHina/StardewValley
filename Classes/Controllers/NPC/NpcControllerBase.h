#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include "Game/Inventory.h"

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

  void update(const cocos2d::Vec2& player_pos) override;

  void handleTalkAt(const cocos2d::Vec2& player_pos) override;

 private:
  std::vector<std::unique_ptr<NpcControllerBase>> controllers_;
};

} // namespace Controllers
