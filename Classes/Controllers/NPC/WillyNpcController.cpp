#include "Controllers/NPC/WillyNpcController.h"
#include "Controllers/UI/UIController.h"
#include "Game/WorldState.h"
#include "Game/NPC/WillyNpc.h"

namespace Controllers {

WillyNpcController::WillyNpcController(
    IMapController* map,
    cocos2d::Node* world_node,
    UIController* ui,
    std::shared_ptr<Game::Inventory> inventory,
    NpcDialogueManager* dialogue)
    : map_(map),
      world_node_(world_node),
      ui_(ui),
      inventory_(std::move(inventory)),
      dialogue_(dialogue) {
  if (!map_ || !world_node_) return;
  cocos2d::Size size = map_->getContentSize();
  float tile = map_->tileSize();
  cocos2d::Vec2 center(size.width * 0.5f, size.height * 0.5f);
  npc_ = std::make_unique<Game::Willy>();
  if (!npc_) return;
  auto sprite = cocos2d::Sprite::create(npc_->texturePath());
  if (!sprite) return;
  float offset_x = 13.5f * tile;
  float offset_y = -11.0f * tile;
  cocos2d::Vec2 pos(center.x + offset_x, center.y + offset_y);
  sprite->setPosition(pos);
  map_->addActorToMap(sprite, 22);
  sprite_ = sprite;
}

bool WillyNpcController::isNear(const cocos2d::Vec2& player_pos,
                                float max_dist,
                                cocos2d::Vec2& out_pos) const {
  if (!sprite_ || !map_) return false;
  cocos2d::Vec2 local = sprite_->getPosition();
  float dist = player_pos.distance(local);
  if (dist >= max_dist) return false;
  cocos2d::Vec2 origin = map_->getOrigin();
  auto size = sprite_->getContentSize();
  float x = origin.x + local.x;
  float y = origin.y + local.y + size.height * 0.5f;
  out_pos = cocos2d::Vec2(x, y);
  return true;
}

int WillyNpcController::friendshipGainForGift() const {
  if (!inventory_ || !npc_) return 0;
  if (inventory_->size() <= 0) return 0;
  if (inventory_->selectedKind() != Game::SlotKind::Item) return 0;
  const auto& slot = inventory_->selectedSlot();
  if (slot.itemQty <= 0) return 0;
  int gained = npc_->friendshipGainForGift(slot.itemType);
  if (gained <= 0) return 0;
  bool consumed = inventory_->consumeSelectedItem(1);
  if (!consumed) return 0;
  return gained;
}

void WillyNpcController::update(const cocos2d::Vec2& player_pos) {
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

void WillyNpcController::handleTalkAt(const cocos2d::Vec2& player_pos) {
  if (!ui_ || !map_) return;
  float max_dist = map_->tileSize() * 1.5f;
  cocos2d::Vec2 pos;
  bool is_near = isNear(player_pos, max_dist, pos);
  if (!is_near || !npc_) return;
  int key = 2;
  const char* npc_name = npc_ ? npc_->name() : "NPC";
  bool hasItem = inventory_ && inventory_->size() > 0 &&
                 inventory_->selectedKind() == Game::SlotKind::Item &&
                 inventory_->selectedSlot().itemQty > 0;
  if (!hasItem) {
    if (dialogue_) dialogue_->startDialogue(key, npc_name);
    return;
  }
  auto& ws = Game::globalState();
  int current = 0;
  auto it = ws.npcFriendship.find(key);
  if (it != ws.npcFriendship.end()) current = it->second;
  int today = ws.seasonIndex * 30 + ws.dayOfSeason;
  auto itDay = ws.npcLastGiftDay.find(key);
  bool giftedToday = (itDay != ws.npcLastGiftDay.end() && itDay->second == today);
  if (giftedToday) {
    float tile = map_->tileSize();
    if (ui_) {
      ui_->popFriendshipTextAt(pos + cocos2d::Vec2(0, 0.5f * tile),
                               "Already gifted today",
                               cocos2d::Color3B::YELLOW);
    }
    return;
  }
  int gained = friendshipGainForGift();
  if (gained <= 0) return;
  ws.npcLastGiftDay[key] = today;
  if (ui_) ui_->refreshHotbar();
  int next = current + gained;
  if (next > 250) next = 250;
  ws.npcFriendship[key] = next;
  std::string text =
      std::string(npc_name) + ": Friendship " + std::to_string(next);
  float tile = map_->tileSize();
  ui_->popFriendshipTextAt(pos + cocos2d::Vec2(0, 0.5f * tile),
                           text,
                           cocos2d::Color3B::WHITE);
}

bool WillyNpcController::handleRightClick(cocos2d::EventMouse* e) {
  if (!ui_ || !sprite_) return false;
  if (e->getMouseButton() != cocos2d::EventMouse::MouseButton::BUTTON_RIGHT)
    return false;
  cocos2d::Vec2 click = e->getLocation();
  cocos2d::Node* parent = sprite_->getParent();
  if (!parent) return false;
  cocos2d::Vec2 local = parent->convertToNodeSpace(click);
  cocos2d::Rect box = sprite_->getBoundingBox();
  if (!box.containsPoint(local)) return false;
  int key = 2;
  const char* npc_name = npc_ ? npc_->name() : "NPC";
  ui_->showNpcSocial(key, npc_name);
  return true;
}

}  // namespace Controllers

