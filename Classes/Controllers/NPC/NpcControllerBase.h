#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include "Game/Inventory.h"
#include "Controllers/NPC/NpcDialogueManager.h"

namespace Controllers {

class UIController;

class NpcControllerBase {
 public:
  virtual ~NpcControllerBase() = default;
  virtual void update(const cocos2d::Vec2& player_pos) = 0;
  virtual void handleTalkAt(const cocos2d::Vec2& player_pos) = 0;
  virtual bool handleRightClick(cocos2d::EventMouse* e) { return false; }
  virtual bool advanceDialogueIfActive() { return false; }
};

class NpcController : public NpcControllerBase {
 public:
  explicit NpcController(UIController* ui);

  void add(std::unique_ptr<NpcControllerBase> controller);

  void update(const cocos2d::Vec2& player_pos) override;

  void handleTalkAt(const cocos2d::Vec2& player_pos) override;
  bool handleRightClick(cocos2d::EventMouse* e) override;
  bool advanceDialogueIfActive() override;

  NpcDialogueManager* dialogue() { return &dialogue_; }

 private:
  std::vector<std::unique_ptr<NpcControllerBase>> controllers_;
  NpcDialogueManager dialogue_;
};

} // namespace Controllers
