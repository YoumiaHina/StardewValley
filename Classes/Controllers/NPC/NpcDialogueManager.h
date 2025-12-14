#pragma once

#include "cocos2d.h"
#include <string>
#include <vector>

namespace Controllers {

class UIController;

struct DialogueOption {
  std::string text;
  int nextId = 0;
};

struct DialogueNode {
  int id = 0;
  std::string text;
  std::vector<DialogueOption> options;
  bool terminal = false;
};

class NpcDialogueManager {
 public:
  explicit NpcDialogueManager(UIController* ui) : ui_(ui) {}

  void startDialogue(int npcKey, const std::string& npcName);
  bool isActive() const { return active_; }
  void advance();
  void selectOption(int index);
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

