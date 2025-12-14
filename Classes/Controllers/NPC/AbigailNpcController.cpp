#include "Controllers/NPC/AbigailNpcController.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/UI/UIController.h"
#include "Game/WorldState.h"
#include "Game/NPC/AbigailNpc.h"

namespace Controllers {

AbigailNpcController::AbigailNpcController(
    TownMapController* map,
    cocos2d::Node* world_node,
    UIController* ui,
    std::shared_ptr<Game::Inventory> inventory)
    : map_(map),
      world_node_(world_node),
      ui_(ui),
      inventory_(std::move(inventory)) {
  if (!map_ || !world_node_) return;
  cocos2d::Size size = map_->getContentSize();
  float tile = map_->tileSize();
  cocos2d::Vec2 center(size.width * 0.5f, size.height * 0.5f);
  npc_ = std::make_unique<Game::Abigail>();
  if (!npc_) return;
  auto sprite = cocos2d::Sprite::create(npc_->texturePath());
  if (!sprite) return;
  float offset_x = -2.0f * tile;
  cocos2d::Vec2 pos(center.x + offset_x, center.y);
  sprite->setPosition(pos);
  map_->addActorToMap(sprite, 22);
  sprite_ = sprite;
}

bool AbigailNpcController::isNear(const cocos2d::Vec2& player_pos,
                                  float max_dist,
                                  cocos2d::Vec2& out_pos) const {
  if (!sprite_ || !map_) return false;
  cocos2d::Vec2 local = sprite_->getPosition();
  float dist = player_pos.distance(local);
  if (dist >= max_dist) return false;
  cocos2d::Node* parent = sprite_->getParent();
  cocos2d::Vec2 world =
      parent ? parent->convertToWorldSpace(local) : local;
  auto size = sprite_->getContentSize();
  out_pos = cocos2d::Vec2(world.x, world.y + size.height * 0.5f);
  return true;
}

int AbigailNpcController::friendshipGainForGift() const {
  int gained = 5;
  if (!inventory_ || !npc_) return gained;
  if (inventory_->size() <= 0) return gained;
  if (inventory_->selectedKind() != Game::SlotKind::Item) return gained;
  const auto& slot = inventory_->selectedSlot();
  if (slot.itemQty <= 0) return gained;
  gained = npc_->friendshipGainForGift(slot.itemType);
  bool consumed = inventory_->consumeSelectedItem(1);
  if (!consumed) gained = 5;
  return gained;
}

void AbigailNpcController::update(const cocos2d::Vec2& player_pos) {
  if (!ui_ || !map_) return;
  float max_dist = map_->tileSize() * 1.5f;
  cocos2d::Vec2 pos;
  bool is_near = isNear(player_pos, max_dist, pos);
  if (is_near) {
    ui_->showNpcPrompt(true, pos, "Space to Talk / Give Gift");
    was_near_ = true;
  } else if (was_near_) {
    ui_->showNpcPrompt(false, cocos2d::Vec2::ZERO, "");
    was_near_ = false;
  }
}

void AbigailNpcController::handleTalkAt(const cocos2d::Vec2& player_pos) {
  if (!ui_ || !map_) return;
  float max_dist = map_->tileSize() * 1.5f;
  cocos2d::Vec2 pos;
  bool is_near = isNear(player_pos, max_dist, pos);
  if (!is_near || !npc_) return;
  auto& ws = Game::globalState();
  int key = 1;
  int current = 0;
  auto it = ws.npcFriendship.find(key);
  if (it != ws.npcFriendship.end()) current = it->second;
  int gained = friendshipGainForGift();
  int next = current + gained;
  if (next > 250) next = 250;
  ws.npcFriendship[key] = next;
  const char* npc_name = npc_ ? npc_->name() : "NPC";
  std::string text =
      std::string(npc_name) + ": Friendship " + std::to_string(next);
  ui_->popTextAt(pos + cocos2d::Vec2(0, 24.0f), text, cocos2d::Color3B::WHITE);
}

}  // namespace Controllers
