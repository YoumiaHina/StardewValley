#pragma once

#include "cocos2d.h"
#include <string>
#include <vector>

namespace Controllers {

class UIController;

// 对话选项数据：
// - text：显示给玩家的选项文字。
// - nextId：被选择后跳转到的下一个对话节点 id（0 表示结束）。
struct DialogueOption {
  std::string text;
  int nextId = 0;
};

// 单个对话节点：
// - id：对话节点唯一编号。
// - text：当前 NPC 台词。
// - options：可选项列表（为空时视为无选项）。
// - terminal：标记为终结节点（无后续内容，自动关闭对话）。
struct DialogueNode {
  int id = 0;
  std::string text;
  std::vector<DialogueOption> options;
  bool terminal = false;
};

// NPC 对话管理器：
// - 按 npcKey 选择不同 NPC 的对话树（Abigail/Willy 等）。
// - 负责通过 UIController 显示当前台词与选项，并响应玩家选择。
class NpcDialogueManager {
 public:
  explicit NpcDialogueManager(UIController* ui) : ui_(ui) {}

  // 开始一个 NPC 对话：根据 npcKey 选择起始节点并立即展示。
  void startDialogue(int npcKey, const std::string& npcName);
  bool isActive() const { return active_; }
  // 在“无选项”的节点上调用，用于推进到下一句或结束对话。
  void advance();
  // 处理玩家点击的某个选项（按 options 下标）。
  void selectOption(int index);
  // 关闭当前对话并通知 UI 隐藏对话框。
  void close();

 private:
  UIController* ui_ = nullptr;
  bool active_ = false;
  int npcKey_ = 0;
  int currentId_ = 0;
  std::string npcName_;

  DialogueNode nodeFor(int npcKey, int id) const;
  int firstNodeFor(int npcKey) const;
  void presentCurrent();
};

}  // namespace Controllers

