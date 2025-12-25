#pragma once

#include <memory>
#include "cocos2d.h"
#include "Controllers/NPC/NpcControllerBase.h"
#include "Game/Inventory.h"
#include "Game/NPC/NpcBase.h"

namespace Controllers {

class FarmMapController;
class UIController;
class AnimalSystem;

// Robin NPC 控制器（农场建筑/动物商贩）：
// - 控制 Robin 在农场地图上的出生位置与巡逻路径。
// - 负责与玩家的对话提示，以及打开购买动物的商店面板。
// - 协调 AnimalSystem，用于后续创建动物实例。
class RobinNpcController : public NpcControllerBase {
 public:
  // 构造：创建 Robin 精灵并添加到农场地图，初始化巡逻。
  RobinNpcController(FarmMapController* map,
                     cocos2d::Node* world_node,
                     UIController* ui,
                     std::shared_ptr<Game::Inventory> inventory,
                     AnimalSystem* animals);
  ~RobinNpcController() override;

  // 每帧更新：显示“购买动物”提示，并在 UI 打开时暂停巡逻。
  void update(const cocos2d::Vec2& player_pos) override;

  // 处理空间键交互：弹出简单对话，并在选项中打开动物商店。
  void handleTalkAt(const cocos2d::Vec2& player_pos) override;
  // 处理右键点击：直接打开动物商店 UI。
  bool handleRightClick(cocos2d::EventMouse* e) override;

 private:
  FarmMapController* map_ = nullptr;
  cocos2d::Node* world_node_ = nullptr;
  UIController* ui_ = nullptr;
  std::shared_ptr<Game::Inventory> inventory_;
  std::unique_ptr<Game::NpcBase> npc_;
  cocos2d::Sprite* sprite_ = nullptr;
  bool was_near_ = false;
  bool paused_by_interaction_ = false;
  AnimalSystem* animals_ = nullptr;

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
