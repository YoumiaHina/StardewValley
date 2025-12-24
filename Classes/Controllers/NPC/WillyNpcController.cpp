#include "Controllers/NPC/WillyNpcController.h"
#include "Controllers/UI/UIController.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"
#include "Game/NPC/WillyNpc.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace Controllers {

namespace {

// 方向枚举：用于决定动画序列与静态贴图。
constexpr int kDirDown = 0;
constexpr int kDirRight = 1;
constexpr int kDirUp = 2;
constexpr int kDirLeft = 3;

// 行为标签：区分“行走动画”和“巡逻路径”两个 Action。
constexpr int kWalkActionTag = 21001;
constexpr int kPatrolActionTag = 21002;

// 根据方向返回用于拼接贴图路径的 token。
std::string dirToken(int dir) {
  switch (dir) {
    case kDirUp: return "up";
    case kDirLeft: return "left";
    case kDirRight: return "right";
    case kDirDown:
    default: return "downwards";
  }
}

// 根据位移向量估算 NPC 面朝的方向。
int dirFromDelta(const cocos2d::Vec2& d) {
  float ax = std::abs(d.x);
  float ay = std::abs(d.y);
  if (ax > ay) return d.x >= 0 ? kDirRight : kDirLeft;
  return d.y >= 0 ? kDirUp : kDirDown;
}

// 构建指定 NPC 在某个方向上的行走动画帧序列。
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

// 根据 NPC 名称、方向与帧号生成立绘贴图路径。
std::string framePath(const std::string& namePrefix, int dir, int frameIndex1Based) {
  std::string token = dirToken(dir);
  return "NPC/" + namePrefix + "/" + namePrefix + "-" + token + "-" + std::to_string(frameIndex1Based) + ".png";
}

}  // namespace

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
  // 基于地图中心和偏移量，计算 Willy 的出生位置，并尝试微调避免与障碍重叠。
  if (!map_ || !world_node_) return;
  cocos2d::Size size = map_->getContentSize();
  tile_ = map_->tileSize();
  if (tile_ <= 0.0f) tile_ = static_cast<float>(GameConfig::TILE_SIZE);
  cocos2d::Vec2 center(size.width * 0.5f, size.height * 0.5f);
  npc_ = std::make_unique<Game::Willy>();
  if (!npc_) return;
  auto sprite = cocos2d::Sprite::create(framePath("Willy", kDirDown, 1));
  if (!sprite) return;
  if (sprite->getTexture()) sprite->getTexture()->setAliasTexParameters();
  float offset_x = 13.5f * tile_;
  float offset_y = -11.0f * tile_;
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
  // 把 Willy 精灵添加到地图图层，并记录为后续巡逻的基准点。
  sprite->setPosition(pos);
  map_->addActorToMap(sprite, 22);
  sprite_ = sprite;
  patrol_base_pos_ = pos;

  walk_down_ = buildWalkAnim("Willy", kDirDown);
  walk_up_ = buildWalkAnim("Willy", kDirUp);
  walk_left_ = buildWalkAnim("Willy", kDirLeft);
  walk_right_ = buildWalkAnim("Willy", kDirRight);
  CC_SAFE_RETAIN(walk_down_);
  CC_SAFE_RETAIN(walk_up_);
  CC_SAFE_RETAIN(walk_left_);
  CC_SAFE_RETAIN(walk_right_);

  setStanding(kDirDown);
  startPatrol();
}

WillyNpcController::~WillyNpcController() {
  CC_SAFE_RELEASE(walk_down_);
  CC_SAFE_RELEASE(walk_up_);
  CC_SAFE_RELEASE(walk_left_);
  CC_SAFE_RELEASE(walk_right_);
}

// 计算玩家是否在指定距离内，并返回一个适合显示 UI 提示的世界坐标。
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

// 从当前背包选中物品中计算赠礼带来的好感增量，并在成功时消耗 1 个物品。
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
  // 依据地图环境层高度调整绘制顺序，让 NPC 正确被环境遮挡。
  if (sprite_) map_->sortActorWithEnvironment(sprite_);
  // 当任意 UI 面板打开时，暂停巡逻并隐藏交互提示。
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
  // UI 关闭后恢复巡逻。
  if (paused_by_interaction_) {
    startPatrol();
    paused_by_interaction_ = false;
  }
  // 玩家靠近指定半径时显示“对话/赠礼”提示。
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

// 为 Willy 构建一个循环巡逻路线，遇到碰撞会由地图帮助调整目的地。
void WillyNpcController::startPatrol() {
  if (!sprite_ || !map_) return;
  sprite_->stopActionByTag(kPatrolActionTag);

  const float speed = tile_ * 1.0f;
  const float pause = 5.0f;
  const float radius = 8.0f;

  // 辅助函数：根据一组相对偏移构建一串“走到目标+站立+停顿”的动作序列。
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

  // 优先尝试一条较长的路径，不可行时再退化为短路线。
  cocos2d::Vector<cocos2d::FiniteTimeAction*> actions = buildPatrolActions({
      cocos2d::Vec2(2.0f, 0.0f),
      cocos2d::Vec2(2.0f, 2.0f),
      cocos2d::Vec2(0.0f, 2.0f),
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
void WillyNpcController::playWalk(int dir) {
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

// 切换到静止站立帧，同时记录 NPC 面朝方向。
void WillyNpcController::setStanding(int dir) {
  if (!sprite_) return;
  sprite_->stopActionByTag(kWalkActionTag);
  facing_dir_ = dir;
  std::string path = framePath("Willy", dir, 1);
  sprite_->setTexture(path);
  if (sprite_->getTexture()) sprite_->getTexture()->setAliasTexParameters();
}

// 空间键交互：若手上无礼物则开启对话，有礼物则尝试赠礼并更新好感。
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
  // 没有可赠送的物品时，仅触发对话树。
  if (!hasItem) {
    if (dialogue_) dialogue_->startDialogue(key, npc_name);
    return;
  }
  // 有礼物时，根据当日是否已赠礼决定是否允许再次送礼并结算好感。
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
  // 好感值上限 250，更新后在头顶弹出当前好感数字。
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

// 右键点击 Willy 精灵时，打开对应的社交面板。
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
