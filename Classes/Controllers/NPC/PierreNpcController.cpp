#include "Controllers/NPC/PierreNpcController.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/UI/UIController.h"
#include "Game/GameConfig.h"
#include "Game/NPC/PierreNpc.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace Controllers {

namespace {

// 方向枚举：用于决定 Pierre 的面朝方向与贴图选择。
constexpr int kDirDown = 0;
constexpr int kDirRight = 1;
constexpr int kDirUp = 2;
constexpr int kDirLeft = 3;

// 行为标签：区分“行走动画”和“巡逻路径”两个动作。
constexpr int kWalkActionTag = 21001;
constexpr int kPatrolActionTag = 21002;

// 根据方向返回用于拼接贴图路径的方向 token。
std::string dirToken(int dir) {
  switch (dir) {
    case kDirUp: return "up";
    case kDirLeft: return "left";
    case kDirRight: return "right";
    case kDirDown:
    default: return "downwards";
  }
}

// 根据位移向量估算当前行走方向。
int dirFromDelta(const cocos2d::Vec2& d) {
  float ax = std::abs(d.x);
  float ay = std::abs(d.y);
  if (ax > ay) return d.x >= 0 ? kDirRight : kDirLeft;
  return d.y >= 0 ? kDirUp : kDirDown;
}

// 构建 Pierre 在某个方向上的循环行走动画。
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

// 根据 NPC 名称、方向与帧号生成静止帧贴图路径。
std::string framePath(const std::string& namePrefix, int dir, int frameIndex1Based) {
  std::string token = dirToken(dir);
  return "NPC/" + namePrefix + "/" + namePrefix + "-" + token + "-" + std::to_string(frameIndex1Based) + ".png";
}

}  // namespace

// 初始化 Pierre：在地图上放置精灵，修正出生点避免碰撞，并构建行走动画与巡逻基准位置。
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
  tile_ = map_->tileSize();
  if (tile_ <= 0.0f) tile_ = static_cast<float>(GameConfig::TILE_SIZE);
  cocos2d::Vec2 center(size.width * 0.5f, size.height * 0.5f);
  npc_ = std::make_unique<Game::Pierre>();
  if (!npc_) return;
  auto sprite = cocos2d::Sprite::create(framePath("Pierre", kDirDown, 1));
  if (!sprite) return;
  if (sprite->getTexture()) sprite->getTexture()->setAliasTexParameters();
  float offset_x = -7.0f * tile_;
  float offset_y = 5.5f * tile_;
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

  walk_down_ = buildWalkAnim("Pierre", kDirDown);
  walk_up_ = buildWalkAnim("Pierre", kDirUp);
  walk_left_ = buildWalkAnim("Pierre", kDirLeft);
  walk_right_ = buildWalkAnim("Pierre", kDirRight);
  CC_SAFE_RETAIN(walk_down_);
  CC_SAFE_RETAIN(walk_up_);
  CC_SAFE_RETAIN(walk_left_);
  CC_SAFE_RETAIN(walk_right_);

  setStanding(kDirDown);
  startPatrol();
}

// 释放 Pierre 行走动画的引用计数。
PierreNpcController::~PierreNpcController() {
  CC_SAFE_RELEASE(walk_down_);
  CC_SAFE_RELEASE(walk_up_);
  CC_SAFE_RELEASE(walk_left_);
  CC_SAFE_RELEASE(walk_right_);
}

// 计算玩家是否在指定交互距离内，并返回一个适合显示 UI 提示的世界坐标。
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

// 每帧更新：在有交互 UI 打开时暂停巡逻，否则根据玩家距离显示“对话/开店”提示。
void PierreNpcController::update(const cocos2d::Vec2& player_pos) {
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
    ui_->showNpcPrompt(true, pos, "Space to Talk / Open Store");
    was_near_ = true;
  } else if (was_near_) {
    ui_->showNpcPrompt(false, cocos2d::Vec2::ZERO, "");
    was_near_ = false;
  }
}

// 为 Pierre 构建一条循环巡逻路线，并借助地图碰撞信息修正每个路点的位置。
void PierreNpcController::startPatrol() {
  if (!sprite_ || !map_) return;
  sprite_->stopActionByTag(kPatrolActionTag);

  const float speed = tile_ * 1.0f;
  const float pause = 5.0f;
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
      cocos2d::Vec2(0.0f, 0.0f),
      cocos2d::Vec2(5.0f, 0.0f),
      cocos2d::Vec2(-8.0f, 0.0f),
      cocos2d::Vec2(0.0f, 0.0f),
  });
  if (actions.empty()) {
    actions = buildPatrolActions({
        cocos2d::Vec2(1.0f, 0.0f),
        cocos2d::Vec2(1.0f, 1.0f),
        cocos2d::Vec2(0.0f, 1.0f),
        cocos2d::Vec2(0.0f, 0.0f),
    });
  }

  if (actions.empty()) return;
  auto seq = cocos2d::Sequence::create(actions);
  auto rep = cocos2d::RepeatForever::create(seq);
  rep->setTag(kPatrolActionTag);
  sprite_->runAction(rep);
}

// 根据方向播放对应的循环行走动画，并记录当前朝向。
void PierreNpcController::playWalk(int dir) {
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

// 切换到静止帧，让 Pierre 面向给定方向站立。
void PierreNpcController::setStanding(int dir) {
  if (!sprite_) return;
  sprite_->stopActionByTag(kWalkActionTag);
  facing_dir_ = dir;
  std::string path = framePath("Pierre", dir, 1);
  sprite_->setTexture(path);
  if (sprite_->getTexture()) sprite_->getTexture()->setAliasTexParameters();
}

// 空间键交互：弹出对话框，让玩家选择是否打开杂货店。
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

// 右键直接点击 Pierre 精灵时，立即打开杂货店面板。
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
