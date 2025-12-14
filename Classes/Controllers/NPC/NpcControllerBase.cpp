#include "Controllers/NPC/NpcControllerBase.h"
#include "Controllers/NPC/AbigailNpcController.h"
#include "Controllers/NPC/WillyNpcController.h"

namespace Controllers {

TownNpcController::TownNpcController(TownMapController* map,
                                     cocos2d::Node* world_node,
                                     UIController* ui,
                                     std::shared_ptr<Game::Inventory> inventory)
    : dialogue_(ui) {
  controllers_.push_back(
      std::make_unique<AbigailNpcController>(map, world_node, ui, inventory, &dialogue_));
  controllers_.push_back(
      std::make_unique<WillyNpcController>(map, world_node, ui, inventory, &dialogue_));
}

void TownNpcController::update(const cocos2d::Vec2& player_pos) {
  for (auto& c : controllers_) {
    if (c) c->update(player_pos);
  }
}

void TownNpcController::handleTalkAt(const cocos2d::Vec2& player_pos) {
  for (auto& c : controllers_) {
    if (c) c->handleTalkAt(player_pos);
  }
}

bool TownNpcController::advanceDialogueIfActive() {
  if (!dialogue_.isActive()) return false;
  dialogue_.advance();
  return true;
}

void TownNpcController::startDialogueFor(int npcKey, const std::string& npcName) {
  dialogue_.startDialogue(npcKey, npcName);
}

bool TownNpcController::handleRightClick(cocos2d::EventMouse* e) {
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
