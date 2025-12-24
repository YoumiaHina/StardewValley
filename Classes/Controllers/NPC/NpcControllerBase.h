#pragma once

#include "cocos2d.h"
#include <vector>
#include <memory>
#include "Game/Inventory.h"
#include "Controllers/NPC/NpcDialogueManager.h"

namespace Controllers {

class UIController;

// NPC 控制器基类：
// - 抽象出 NPC 行为更新与交互接口。
// - 具体 NPC 控制器负责在 update/handleTalkAt 等函数中实现逻辑。
class NpcControllerBase {
 public:
  // 每帧更新：根据玩家位置驱动 NPC 巡逻、朝向等行为。
  virtual ~NpcControllerBase() = default;
  // 处理与玩家距离相关的逻辑（如提示、排序、停走等）。
  virtual void update(const cocos2d::Vec2& player_pos) = 0;
  // 当玩家在邻近格按下对话键时触发，负责打开对话或处理赠礼。
  virtual void handleTalkAt(const cocos2d::Vec2& player_pos) = 0;
  // 鼠标右键交互入口：默认不处理，具体 NPC 可用来打开社交面板/商店等。
  virtual bool handleRightClick(cocos2d::EventMouse* e) { return false; }
  // 若当前有激活的对话，则推进一轮对话；默认返回 false。
  virtual bool advanceDialogueIfActive() { return false; }
};

// NPC 控制器聚合器：
// - 管理一组具体 NPC 控制器（Abigail/Pierre/Robin/Willy 等）。
// - 统一转发 update/对话/鼠标事件，并维护通用对话管理器。
class NpcController : public NpcControllerBase {
 public:
  // 构造：保存 UI 指针并初始化内部对话管理器。
  explicit NpcController(UIController* ui);

  // 注册一个具体 NPC 控制器；空指针将被忽略。
  void add(std::unique_ptr<NpcControllerBase> controller);

  // 将玩家位置广播给所有子控制器。
  void update(const cocos2d::Vec2& player_pos) override;

  // 将对话请求转发给所有子控制器。
  void handleTalkAt(const cocos2d::Vec2& player_pos) override;
  // 让所有子控制器有机会消费右键事件；任一返回 true 即终止遍历。
  bool handleRightClick(cocos2d::EventMouse* e) override;
  // 若内部对话管理器当前有对话，则推进到下一轮。
  bool advanceDialogueIfActive() override;

  // 访问内部对话管理器，可用于手动启动 NPC 对话。
  NpcDialogueManager* dialogue() { return &dialogue_; }

 private:
  std::vector<std::unique_ptr<NpcControllerBase>> controllers_;
  NpcDialogueManager dialogue_;
};

} // namespace Controllers
