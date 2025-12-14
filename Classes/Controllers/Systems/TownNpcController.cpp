#include "Controllers/Systems/TownNpcController.h"

#include "Controllers/Map/TownMapController.h"
#include "Controllers/UI/UIController.h"
#include "Game/Npc.h"
#include "Game/WorldState.h"

namespace Controllers {

TownNpcController::TownNpcController(TownMapController* map,
                                     cocos2d::Node* world_node,
                                     UIController* ui,
                                     std::shared_ptr<Game::Inventory> inventory)
    : map_(map),
      world_node_(world_node),
      ui_(ui),
      inventory_(std::move(inventory)) {
  buildInitialNpcs();
}

void TownNpcController::buildInitialNpcs() {
  if (!map_ || !world_node_) return;
  cocos2d::Size size = map_->getContentSize();
  float tile = map_->tileSize();
  cocos2d::Vec2 center(size.width * 0.5f, size.height * 0.5f);
  const auto& ids = Game::NpcRegistry::allIds();
  for (std::size_t i = 0; i < ids.size(); ++i) {
    Game::NpcId id = ids[i];
    std::unique_ptr<Game::NpcBase> npc = Game::NpcRegistry::create(id);
    if (!npc) continue;
    auto sprite = cocos2d::Sprite::create(npc->texturePath());
    if (!sprite) continue;
    float offset_x = (i == 0) ? -2.0f * tile : 2.0f * tile;
    cocos2d::Vec2 pos(center.x + offset_x, center.y);
    sprite->setPosition(pos);
    map_->addActorToMap(sprite, 22);
    NpcInstance inst;
    inst.id = id;
    inst.npc = std::move(npc);
    inst.sprite = sprite;
    npcs_.push_back(std::move(inst));
  }
}

bool TownNpcController::findNearestNpc(const cocos2d::Vec2& player_pos,
                                       float max_dist,
                                       int& out_index,
                                       cocos2d::Vec2& out_pos) const {
  float best = max_dist;
  bool found = false;
  out_index = -1;
  for (int i = 0; i < static_cast<int>(npcs_.size()); ++i) {
    const auto& n = npcs_[i];
    if (!n.sprite) continue;
    cocos2d::Vec2 local = n.sprite->getPosition();
    float dist = player_pos.distance(local);
    if (dist < best) {
      best = dist;
      out_index = i;
      cocos2d::Node* parent = n.sprite->getParent();
      cocos2d::Vec2 world =
          parent ? parent->convertToWorldSpace(local) : local;
      auto size = n.sprite->getContentSize();
      out_pos = cocos2d::Vec2(world.x, world.y + size.height * 0.5f);
      found = true;
    }
  }
  return found;
}

int TownNpcController::friendshipGainForGift(NpcInstance& inst) const {
  int gained = 5;
  if (!inventory_ || !inst.npc) return gained;
  if (inventory_->size() <= 0) return gained;
  if (inventory_->selectedKind() != Game::SlotKind::Item) return gained;
  const auto& slot = inventory_->selectedSlot();
  if (slot.itemQty <= 0) return gained;
  gained = inst.npc->friendshipGainForGift(slot.itemType);
  bool consumed = inventory_->consumeSelectedItem(1);
  if (!consumed) gained = 5;
  return gained;
}

void TownNpcController::update(const cocos2d::Vec2& player_pos) {
  if (!ui_ || !map_) return;
  float max_dist = map_->tileSize() * 1.5f;
  int index = -1;
  cocos2d::Vec2 pos;
  bool is_near = findNearestNpc(player_pos, max_dist, index, pos);
  if (is_near) {
    ui_->showNpcPrompt(true, pos, "Space to Talk / Give Gift");
  } else {
    ui_->showNpcPrompt(false, cocos2d::Vec2::ZERO, "");
  }
}

void TownNpcController::handleTalkAt(const cocos2d::Vec2& player_pos) {
  if (!ui_ || !map_) return;
  float max_dist = map_->tileSize() * 1.5f;
  int index = -1;
  cocos2d::Vec2 pos;
  bool is_near = findNearestNpc(player_pos, max_dist, index, pos);
  if (!is_near || index < 0 || index >= static_cast<int>(npcs_.size())) return;
  auto& inst = npcs_[index];
  auto& ws = Game::globalState();
  int key = static_cast<int>(inst.id);
  int current = 0;
  auto it = ws.npcFriendship.find(key);
  if (it != ws.npcFriendship.end()) current = it->second;
  int gained = friendshipGainForGift(inst);
  int next = current + gained;
  if (next > 250) next = 250;
  ws.npcFriendship[key] = next;
  const char* npc_name = inst.npc ? inst.npc->name() : "NPC";
  std::string text =
      std::string(npc_name) + ": Friendship " + std::to_string(next);
  ui_->popTextAt(pos + cocos2d::Vec2(0, 24.0f), text, cocos2d::Color3B::WHITE);
}

}  // namespace Controllers

