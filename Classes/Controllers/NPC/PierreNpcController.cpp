#include "Controllers/NPC/PierreNpcController.h"
#include "Controllers/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Game/NPC/PierreNpc.h"
#include <string>
#include <vector>

namespace Controllers {

PierreNpcController::PierreNpcController(
    IMapController* map,
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
  npc_ = std::make_unique<Game::Pierre>();
  if (!npc_) return;
  auto sprite = cocos2d::Sprite::create(npc_->texturePath());
  if (!sprite) return;
  float offset_x = -7.0f * tile;
  float offset_y = 6.0f * tile;
  cocos2d::Vec2 pos(center.x + offset_x, center.y + offset_y);
  sprite->setPosition(pos);
  map_->addActorToMap(sprite, 22);
  sprite_ = sprite;
}

bool PierreNpcController::isNear(const cocos2d::Vec2& player_pos,
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

void PierreNpcController::update(const cocos2d::Vec2& player_pos) {
  if (!ui_ || !map_) return;
  float max_dist = map_->tileSize() * 1.5f;
  cocos2d::Vec2 pos;
  bool is_near = isNear(player_pos, max_dist, pos);
  if (is_near) {
    ui_->showNpcPrompt(true, pos, "Space to Talk / Open Store");
    was_near_ = true;
  } else if (was_near_) {
    ui_->showNpcPrompt(false, cocos2d::Vec2::ZERO, "");
    was_near_ = false;
  }
}

void PierreNpcController::handleTalkAt(const cocos2d::Vec2& player_pos) {
  if (!ui_ || !map_) return;
  float max_dist = map_->tileSize() * 1.5f;
  cocos2d::Vec2 pos;
  bool is_near = isNear(player_pos, max_dist, pos);
  if (!is_near) return;
  if (ui_->isDialogueVisible()) return;
  std::string name = npc_ ? npc_->name() : "Pierre";
  std::string text = "Welcome. My general store has everything you need.";
  std::vector<std::string> options;
  options.emplace_back("Open Store");
  options.emplace_back("Cancel");
  ui_->showDialogue(
      name,
      text,
      options,
      [this](int index) {
        if (!ui_) return;
        ui_->hideDialogue();
        if (index == 0) {
          ui_->toggleStorePanel(true);
        }
      },
      nullptr);
}

bool PierreNpcController::handleRightClick(cocos2d::EventMouse* e) {
  if (!ui_ || !sprite_) return false;
  if (e->getMouseButton() != cocos2d::EventMouse::MouseButton::BUTTON_RIGHT)
    return false;
  cocos2d::Vec2 click = e->getLocation();
  cocos2d::Node* parent = sprite_->getParent();
  if (!parent) return false;
  cocos2d::Vec2 local = parent->convertToNodeSpace(click);
  cocos2d::Rect box = sprite_->getBoundingBox();
  if (!box.containsPoint(local)) return false;
  ui_->toggleStorePanel(true);
  return true;
}

}  // namespace Controllers
