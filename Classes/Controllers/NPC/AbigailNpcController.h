#pragma once

#include <memory>
#include "cocos2d.h"
#include "Controllers/NPC/NpcControllerBase.h"
#include "Controllers/Map/IMapController.h"
#include "Game/Inventory.h"
#include "Game/NPC/NpcBase.h"

namespace Controllers {

class UIController;

// Abigail NPC 控制器：
// - 负责 Abigail 的出生位置、巡逻行为与朝向切换。
// - 处理与玩家的对话、赠礼与社交面板打开逻辑。
// - 协作对象：地图控制器（寻路/碰撞）、UI 控制器、Inventory 与 NpcDialogueManager。
class AbigailNpcController : public NpcControllerBase {
 public:
  // 构造：在指定地图/world 节点上创建 Abigail 精灵并开始巡逻。
  AbigailNpcController(IMapController* map,
                       cocos2d::Node* world_node,
                       UIController* ui,
                       std::shared_ptr<Game::Inventory> inventory,
                       NpcDialogueManager* dialogue);
  ~AbigailNpcController() override;

  // 每帧更新：根据玩家距离显示交互提示，并在交互时暂停巡逻。
  void update(const cocos2d::Vec2& player_pos) override;

  // 处理空间键交互：触发对话或尝试赠礼并增长好感度。
  void handleTalkAt(const cocos2d::Vec2& player_pos) override;
  // 处理右键点击：在点击精灵时打开 Abigail 社交面板。
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
