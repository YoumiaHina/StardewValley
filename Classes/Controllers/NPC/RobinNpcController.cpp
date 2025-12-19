#include "Controllers/NPC/RobinNpcController.h"
#include "Controllers/Map/FarmMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/AnimalSystem.h"
#include "Game/GameConfig.h"
#include "Game/NPC/RobinNpc.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace Controllers {

namespace {

constexpr int kDirDown = 0;
constexpr int kDirRight = 1;
constexpr int kDirUp = 2;
constexpr int kDirLeft = 3;

constexpr int kWalkActionTag = 21001;
constexpr int kPatrolActionTag = 21002;

std::string dirToken(int dir) {
  switch (dir) {
    case kDirUp: return "up";
    case kDirLeft: return "left";
    case kDirRight: return "right";
    case kDirDown:
    default: return "downwards";
  }
}

int dirFromDelta(const cocos2d::Vec2& d) {
  float ax = std::abs(d.x);
  float ay = std::abs(d.y);
  if (ax > ay) return d.x >= 0 ? kDirRight : kDirLeft;
  return d.y >= 0 ? kDirUp : kDirDown;
}

cocos2d::Animation* buildWalkAnim(const std::string& namePrefix, int dir) {
  auto anim = cocos2d::Animation::create();
  if (!anim) return nullptr;
  std::string token = dirToken(dir);
  for (int i = 1; i <= 4; ++i) {
    std::string path = "NPC/" + namePrefix + "/" + namePrefix + "-" + token + "-" + std::to_string(i) + ".png";
    anim->addSpriteFrameWithFile(path);
  }
  anim->setDelayPerUnit(0.12f);
  return anim;
}

std::string framePath(const std::string& namePrefix, int dir, int frameIndex1Based) {
  std::string token = dirToken(dir);
  return "NPC/" + namePrefix + "/" + namePrefix + "-" + token + "-" + std::to_string(frameIndex1Based) + ".png";
}

}  // namespace

RobinNpcController::RobinNpcController(
    FarmMapController* map,
    cocos2d::Node* world_node,
    UIController* ui,
    std::shared_ptr<Game::Inventory> inventory,
    AnimalSystem* animals)
    : map_(map),
      world_node_(world_node),
      ui_(ui),
      inventory_(std::move(inventory)),
      animals_(animals) {
  if (!map_ || !world_node_) return;
  cocos2d::Size size = map_->getContentSize();
  tile_ = map_->tileSize();
  if (tile_ <= 0.0f) tile_ = static_cast<float>(GameConfig::TILE_SIZE);
  cocos2d::Vec2 center(size.width * 0.5f, size.height * 0.5f);
  npc_ = std::make_unique<Game::Robin>();
  if (!npc_) return;
  auto sprite = cocos2d::Sprite::create(framePath("Robin", kDirDown, 1));
  if (!sprite) return;
  if (sprite->getTexture()) sprite->getTexture()->setAliasTexParameters();
  float offset_x = 5.0f * tile_;
  float offset_y = 12.0f * tile_;
  cocos2d::Vec2 pos(center.x + offset_x, center.y + offset_y);
  {
    const float radius = 8.0f;
    const float s = tile_;
    auto footPos = [s](const cocos2d::Vec2& p) { return p + cocos2d::Vec2(0, -s * 0.5f); };
    if (map_->collides(footPos(pos), radius)) {
      int c0 = 0;
      int r0 = 0;
      map_->worldToTileIndex(pos, c0, r0);
      const int max_ring = 6;
      bool found = false;
      for (int ring = 0; ring <= max_ring && !found; ++ring) {
        for (int dy = -ring; dy <= ring && !found; ++dy) {
          for (int dx = -ring; dx <= ring && !found; ++dx) {
            if (ring > 0 && std::max(std::abs(dx), std::abs(dy)) != ring) continue;
            int c = c0 + dx;
            int r = r0 + dy;
            if (!map_->inBounds(c, r)) continue;
            cocos2d::Vec2 candidate = map_->tileToWorld(c, r);
            if (!map_->collides(footPos(candidate), radius)) {
              pos = candidate;
              found = true;
            }
          }
        }
      }
    }
  }
  sprite->setPosition(pos);
  map_->addActorToMap(sprite, 22);
  sprite_ = sprite;
  patrol_base_pos_ = pos;

  walk_down_ = buildWalkAnim("Robin", kDirDown);
  walk_up_ = buildWalkAnim("Robin", kDirUp);
  walk_left_ = buildWalkAnim("Robin", kDirLeft);
  walk_right_ = buildWalkAnim("Robin", kDirRight);
  CC_SAFE_RETAIN(walk_down_);
  CC_SAFE_RETAIN(walk_up_);
  CC_SAFE_RETAIN(walk_left_);
  CC_SAFE_RETAIN(walk_right_);

  setStanding(kDirDown);
  startPatrol();
}

RobinNpcController::~RobinNpcController() {
  CC_SAFE_RELEASE(walk_down_);
  CC_SAFE_RELEASE(walk_up_);
  CC_SAFE_RELEASE(walk_left_);
  CC_SAFE_RELEASE(walk_right_);
}

bool RobinNpcController::isNear(const cocos2d::Vec2& player_pos,
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

void RobinNpcController::update(const cocos2d::Vec2& player_pos) {
  if (!ui_ || !map_) return;
  if (sprite_) map_->sortActorWithEnvironment(sprite_);
  bool interacting = ui_->isDialogueVisible()
                     || ui_->isNpcSocialVisible()
                     || ui_->isStorePanelVisible()
                     || ui_->isAnimalStorePanelVisible();
  if (interacting) {
    if (!paused_by_interaction_) {
      if (sprite_) {
        sprite_->stopActionByTag(kPatrolActionTag);
        setStanding(facing_dir_);
      }
      paused_by_interaction_ = true;
    }
    if (was_near_) {
      ui_->showNpcPrompt(false, cocos2d::Vec2::ZERO, "");
      was_near_ = false;
    }
    return;
  }
  if (paused_by_interaction_) {
    startPatrol();
    paused_by_interaction_ = false;
  }
  float max_dist = map_->tileSize() * 1.5f;
  cocos2d::Vec2 pos;
  bool is_near = isNear(player_pos, max_dist, pos);
  if (is_near) {
    ui_->showNpcPrompt(true, pos, "Space to Talk / Buy Animals");
    was_near_ = true;
  } else if (was_near_) {
    ui_->showNpcPrompt(false, cocos2d::Vec2::ZERO, "");
    was_near_ = false;
  }
}

void RobinNpcController::startPatrol() {
  if (!sprite_ || !map_) return;
  sprite_->stopActionByTag(kPatrolActionTag);

  const float speed = tile_ * 1.0f;
  const float pause = 0.25f;
  const float radius = 8.0f;

  auto buildPatrolActions = [&](const std::vector<cocos2d::Vec2>& offsets) {
    cocos2d::Vec2 current = sprite_->getPosition();
    cocos2d::Vector<cocos2d::FiniteTimeAction*> actions;
    actions.reserve(static_cast<int>(offsets.size()) * 4);
    for (const auto& off : offsets) {
      cocos2d::Vec2 desired = patrol_base_pos_ + cocos2d::Vec2(off.x * tile_, off.y * tile_);
      cocos2d::Vec2 dest = map_->clampPosition(current, desired, radius);
      cocos2d::Vec2 delta = dest - current;
      if (delta.lengthSquared() < 0.001f) continue;
      int dir = dirFromDelta(delta);
      float duration = speed > 0.0f ? (delta.length() / speed) : 0.0f;
      actions.pushBack(cocos2d::CallFunc::create([this, dir]() { playWalk(dir); }));
      actions.pushBack(cocos2d::MoveTo::create(duration, dest));
      actions.pushBack(cocos2d::CallFunc::create([this, dir]() { setStanding(dir); }));
      actions.pushBack(cocos2d::DelayTime::create(pause));
      current = dest;
    }
    return actions;
  };

  cocos2d::Vector<cocos2d::FiniteTimeAction*> actions = buildPatrolActions({
      cocos2d::Vec2(2.0f, 0.0f),
      cocos2d::Vec2(2.0f, 8.0f),
      cocos2d::Vec2(2.0f, 0.0f),
      cocos2d::Vec2(0.0f, 0.0f),
  });
  if (actions.empty()) {
    actions = buildPatrolActions({
        cocos2d::Vec2(1.0f, 0.0f),
        cocos2d::Vec2(1.0f, 1.0f),
        cocos2d::Vec2(1.0f, 0.0f),
        cocos2d::Vec2(0.0f, 0.0f),
    });
  }

  if (actions.empty()) return;
  auto seq = cocos2d::Sequence::create(actions);
  auto rep = cocos2d::RepeatForever::create(seq);
  rep->setTag(kPatrolActionTag);
  sprite_->runAction(rep);
}

void RobinNpcController::playWalk(int dir) {
  if (!sprite_) return;
  sprite_->stopActionByTag(kWalkActionTag);
  facing_dir_ = dir;

  cocos2d::Animation* anim = nullptr;
  switch (dir) {
    case kDirUp: anim = walk_up_; break;
    case kDirLeft: anim = walk_left_; break;
    case kDirRight: anim = walk_right_; break;
    case kDirDown:
    default: anim = walk_down_; break;
  }
  if (!anim) return;
  auto rep = cocos2d::RepeatForever::create(cocos2d::Animate::create(anim));
  rep->setTag(kWalkActionTag);
  sprite_->runAction(rep);
}

void RobinNpcController::setStanding(int dir) {
  if (!sprite_) return;
  sprite_->stopActionByTag(kWalkActionTag);
  facing_dir_ = dir;
  std::string path = framePath("Robin", dir, 1);
  sprite_->setTexture(path);
  if (sprite_->getTexture()) sprite_->getTexture()->setAliasTexParameters();
}

void RobinNpcController::handleTalkAt(const cocos2d::Vec2& player_pos) {
  if (!ui_ || !map_) return;
  float max_dist = map_->tileSize() * 1.5f;
  cocos2d::Vec2 pos;
  bool is_near = isNear(player_pos, max_dist, pos);
  if (!is_near) return;
  if (ui_->isDialogueVisible()) return;
  std::string name = npc_ ? npc_->name() : "Robin";
  std::string text = "I can help you buy animals for your farm.";
  std::vector<std::string> options;
  options.emplace_back("Open Shop");
  options.emplace_back("Cancel");
  ui_->showDialogue(
      name,
      text,
      options,
      [this](int index) {
        if (!ui_) return;
        ui_->hideDialogue();
        if (index == 0) {
          ui_->toggleAnimalStorePanel(true);
        }
      },
      nullptr);
}

bool RobinNpcController::handleRightClick(cocos2d::EventMouse* e) {
  if (!ui_ || !sprite_) return false;
  if (e->getMouseButton() != cocos2d::EventMouse::MouseButton::BUTTON_RIGHT)
    return false;
  cocos2d::Vec2 click = e->getLocation();
  cocos2d::Node* parent = sprite_->getParent();
  if (!parent) return false;
  cocos2d::Vec2 local = parent->convertToNodeSpace(click);
  cocos2d::Rect box = sprite_->getBoundingBox();
  if (!box.containsPoint(local)) return false;
  ui_->toggleAnimalStorePanel(true);
  return true;
}

}  // namespace Controllers
