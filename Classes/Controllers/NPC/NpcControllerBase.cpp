#include "Controllers/NPC/NpcControllerBase.h"
#include "Controllers/NPC/AbigailNpcController.h"
#include "Controllers/NPC/WillyNpcController.h"

namespace Controllers {

TownNpcController::TownNpcController(TownMapController* map,
                                     cocos2d::Node* world_node,
                                     UIController* ui,
                                     std::shared_ptr<Game::Inventory> inventory) {
  controllers_.push_back(
      std::make_unique<AbigailNpcController>(map, world_node, ui, inventory));
  controllers_.push_back(
      std::make_unique<WillyNpcController>(map, world_node, ui, inventory));
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

}  // namespace Controllers
