#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include "Game/Inventory.h"
#include "Controllers/NPC/NpcDialogueManager.h"

namespace Controllers {

class TownMapController;
class UIController;

class NpcControllerBase {
 public:
  virtual ~NpcControllerBase() = default;
  virtual void update(const cocos2d::Vec2& player_pos) = 0;
  virtual void handleTalkAt(const cocos2d::Vec2& player_pos) = 0;
  virtual bool handleRightClick(cocos2d::EventMouse* e) { return false; }
  virtual bool advanceDialogueIfActive() { return false; }
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
  NpcDialogueManager dialogue_;

 public:
  bool advanceDialogueIfActive();
  void startDialogueFor(int npcKey, const std::string& npcName);
  bool handleRightClick(cocos2d::EventMouse* e);
};

} // namespace Controllers
