#pragma once

#include <memory>
#include "cocos2d.h"
#include "Controllers/NPC/NpcControllerBase.h"
#include "Controllers/Map/IMapController.h"
#include "Game/Inventory.h"
#include "Game/NPC/NpcBase.h"

namespace Controllers {

class UIController;

// Willy NPC 控制器（渔夫）：
// - 控制 Willy 在海滩地图中的出生位置、巡逻与朝向。
// - 处理与玩家的对话、赠礼与社交面板，使用 NpcDialogueManager 管理多段对话。
class WillyNpcController : public NpcControllerBase {
 public:
  // 构造：在地图上放置 Willy 精灵，并开始巡逻。
  WillyNpcController(IMapController* map,
                     cocos2d::Node* world_node,
                     UIController* ui,
                     std::shared_ptr<Game::Inventory> inventory,
                     NpcDialogueManager* dialogue);
  ~WillyNpcController() override;

  // 每帧更新：根据玩家距离显示提示，并在交互中暂停巡逻。
  void update(const cocos2d::Vec2& player_pos) override;

  // 处理空间键交互：触发对话或尝试赠礼并更新好感度。
  void handleTalkAt(const cocos2d::Vec2& player_pos) override;
  // 处理右键点击：打开 Willy 社交面板。
  bool handleRightClick(cocos2d::EventMouse* e) override;

 private:
  IMapController* map_ = nullptr;
  cocos2d::Node* world_node_ = nullptr;
  UIController* ui_ = nullptr;
  std::shared_ptr<Game::Inventory> inventory_;
  std::unique_ptr<Game::NpcBase> npc_;
  cocos2d::Sprite* sprite_ = nullptr;
  bool was_near_ = false;
  bool paused_by_interaction_ = false;
  NpcDialogueManager* dialogue_ = nullptr;

  float tile_ = 0.0f;
  cocos2d::Vec2 patrol_base_pos_;
  cocos2d::Animation* walk_down_ = nullptr;
  cocos2d::Animation* walk_up_ = nullptr;
  cocos2d::Animation* walk_left_ = nullptr;
  cocos2d::Animation* walk_right_ = nullptr;
  int facing_dir_ = 0;

  bool isNear(const cocos2d::Vec2& player_pos,
              float max_dist,
              cocos2d::Vec2& out_pos) const;
  int friendshipGainForGift() const;

  void startPatrol();
  void playWalk(int dir);
  void setStanding(int dir);
};

}  // namespace Controllers
