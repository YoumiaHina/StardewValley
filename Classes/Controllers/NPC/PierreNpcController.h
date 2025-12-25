#pragma once

#include <memory>
#include "cocos2d.h"
#include "Controllers/NPC/NpcControllerBase.h"
#include "Game/Inventory.h"
#include "Game/NPC/NpcBase.h"

namespace Controllers {

class IMapController;
class UIController;

// Pierre NPC 控制器（杂货店老板）：
// - 控制 Pierre 在城镇地图中的出生位置与简单巡逻。
// - 当玩家靠近并交互时，负责弹出“打开商店”对话及商店面板。
class PierreNpcController : public NpcControllerBase {
 public:
  // 构造：在地图上放置 Pierre 精灵，并初始化简单巡逻路径。
  PierreNpcController(IMapController* map,
                      cocos2d::Node* world_node,
                      UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory);
  ~PierreNpcController() override;

  // 每帧更新：维护 NPC 与环境层的排序，并在交互时暂停/恢复巡逻。
  void update(const cocos2d::Vec2& player_pos) override;

  // 处理空间键交互：显示欢迎对话，并在选项中打开商店 UI。
  void handleTalkAt(const cocos2d::Vec2& player_pos) override;
  // 处理右键点击：直接打开商店面板。
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

  void startPatrol();
  void playWalk(int dir);
  void setStanding(int dir);
};

}  // namespace Controllers
