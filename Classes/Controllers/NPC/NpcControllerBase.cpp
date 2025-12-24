#include "Controllers/NPC/NpcControllerBase.h"

namespace Controllers {

NpcController::NpcController(UIController* ui)
    : dialogue_(ui) {
}

void NpcController::add(std::unique_ptr<NpcControllerBase> controller) {
  if (!controller) return;
  controllers_.push_back(std::move(controller));
}

void NpcController::update(const cocos2d::Vec2& player_pos) {
  // 将玩家位置广播给所有注册的 NPC 控制器，让它们自行判断交互距离与提示。
  for (auto& c : controllers_) {
    if (c) c->update(player_pos);
  }
}

void NpcController::handleTalkAt(const cocos2d::Vec2& player_pos) {
  // 空格键交互时，把事件转发给所有 NPC，由各自决定是否响应。
  for (auto& c : controllers_) {
    if (c) c->handleTalkAt(player_pos);
  }
}

bool NpcController::advanceDialogueIfActive() {
  // 若内部对话管理器当前有对话，则尝试向前推进一轮。
  if (!dialogue_.isActive()) return false;
  dialogue_.advance();
  return true;
}

bool NpcController::handleRightClick(cocos2d::EventMouse* e) {
  // 右键事件从前到后询问所有 NPC；一旦被消费就停止继续传递。
  bool handled = false;
  for (auto& c : controllers_) {
    if (c && c->handleRightClick(e)) {
      handled = true;
      break;
    }
  }
  return handled;
}

}  // namespace Controllers
