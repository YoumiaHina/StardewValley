#include "Controllers/NPC/NpcControllerBase.h"

namespace Controllers {

NpcController::NpcController(UIController* ui)
    : dialogue_(ui) {
}

void NpcController::add(std::unique_ptr<NpcControllerBase> controller) {
  if (!controller) return;
  controllers_.push_back(std::move(controller));
}

void NpcController::update(const cocos2d::Vec2& player_pos) {
  for (auto& c : controllers_) {
    if (c) c->update(player_pos);
  }
}

void NpcController::handleTalkAt(const cocos2d::Vec2& player_pos) {
  for (auto& c : controllers_) {
    if (c) c->handleTalkAt(player_pos);
  }
}

bool NpcController::advanceDialogueIfActive() {
  if (!dialogue_.isActive()) return false;
  dialogue_.advance();
  return true;
}

bool NpcController::handleRightClick(cocos2d::EventMouse* e) {
  bool handled = false;
  for (auto& c : controllers_) {
    if (c && c->handleRightClick(e)) {
      handled = true;
      break;
    }
  }
  return handled;
}

}  // namespace Controllers
