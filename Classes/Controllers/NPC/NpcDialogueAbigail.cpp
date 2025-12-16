#include "Controllers/NPC/NpcDialogueManager.h"
#include "Controllers/UI/UIController.h"
#include <cstdlib>

using namespace cocos2d;

namespace Controllers {

int firstNodeForAbigail();
DialogueNode nodeForAbigail(int id);

int firstNodeForWilly();
DialogueNode nodeForWilly(int id);

int NpcDialogueManager::firstNodeFor(int npcKey) const {
  if (npcKey == 1) return firstNodeForAbigail();
  if (npcKey == 2) return firstNodeForWilly();
  return 0;
}

DialogueNode NpcDialogueManager::nodeFor(int npcKey, int id) const {
  if (npcKey == 1) return nodeForAbigail(id);
  if (npcKey == 2) return nodeForWilly(id);
  DialogueNode n;
  return n;
}

void NpcDialogueManager::presentCurrent() {
  if (!ui_ || !active_) return;
  DialogueNode node = nodeFor(npcKey_, currentId_);
  if (node.id == 0) {
    close();
    return;
  }
  std::vector<std::string> optionTexts;
  for (auto& o : node.options) optionTexts.push_back(o.text);
  ui_->showDialogue(
      npcName_,
      node.text,
      optionTexts,
      [this](int index) { selectOption(index); },
      [this]() { advance(); });
}

void NpcDialogueManager::startDialogue(int npcKey, const std::string& npcName) {
  npcKey_ = npcKey;
  npcName_ = npcName;
  currentId_ = firstNodeFor(npcKey_);
  if (currentId_ == 0) return;
  active_ = true;
  presentCurrent();
}

void NpcDialogueManager::advance() {
  if (!active_) return;
  DialogueNode node = nodeFor(npcKey_, currentId_);
  if (!node.options.empty()) return;
  if (node.terminal) {
    close();
    return;
  }
  if (!node.options.empty()) return;
  close();
}

void NpcDialogueManager::selectOption(int index) {
  if (!active_) return;
  DialogueNode node = nodeFor(npcKey_, currentId_);
  if (index < 0 || index >= static_cast<int>(node.options.size())) return;
  int next = node.options[index].nextId;
  if (next == 0) {
    close();
    return;
  }
  currentId_ = next;
  DialogueNode nextNode = nodeFor(npcKey_, currentId_);
  if (nextNode.id == 0) {
    close();
    return;
  }
  if (nextNode.terminal && nextNode.options.empty()) {
    presentCurrent();
    return;
  }
  presentCurrent();
}

void NpcDialogueManager::close() {
  active_ = false;
  currentId_ = 0;
  if (ui_) ui_->hideDialogue();
}

int firstNodeForAbigail() {
  static int variants[] = {1, 10, 20, 30, 40, 50, 60, 70, 80, 90};
  int idx = std::rand() % 10;
  return variants[idx];
}

DialogueNode nodeForAbigail(int id) {
  DialogueNode n;
  if (id == 1) {
    n.id = 1;
    n.text = "Hi, I am Abigail. How is your day?";
    n.options = {{"Pretty good", 2}, {"So-so", 3}};
  } else if (id == 2) {
    n.id = 2;
    n.text = "Glad to hear that.";
    n.terminal = true;
  } else if (id == 3) {
    n.id = 3;
    n.text = "Hope it gets better soon.";
    n.terminal = true;
  } else if (id == 10) {
    n.id = 10;
    n.text = "Lovely weather, isn't it?";
    n.options = {{"Absolutely", 11}, {"Not really", 12}};
  } else if (id == 11) {
    n.id = 11;
    n.text = "Great. Enjoy your day.";
    n.terminal = true;
  } else if (id == 12) {
    n.id = 12;
    n.text = "Maybe a cup of tea would help.";
    n.terminal = true;
  } else if (id == 20) {
    n.id = 20;
    n.text = "Have you visited the mine?";
    n.options = {{"Yes", 21}, {"No", 22}};
  } else if (id == 21) {
    n.id = 21;
    n.text = "Be careful down there.";
    n.terminal = true;
  } else if (id == 22) {
    n.id = 22;
    n.text = "You should, if you feel brave.";
    n.terminal = true;
  } else if (id == 30) {
    n.id = 30;
    n.text = "Have you harvested anything today?";
    n.options = {{"Yes", 31}, {"Not yet", 32}};
  } else if (id == 31) {
    n.id = 31;
    n.text = "Fresh crops always smell nice.";
    n.terminal = true;
  } else if (id == 32) {
    n.id = 32;
    n.text = "Then you should hurry before it gets dark.";
    n.terminal = true;
  } else if (id == 40) {
    n.id = 40;
    n.text = "Do you like this town?";
    n.options = {{"Very much", 41}, {"It is ok", 42}};
  } else if (id == 41) {
    n.id = 41;
    n.text = "I am happy you feel at home.";
    n.terminal = true;
  } else if (id == 42) {
    n.id = 42;
    n.text = "Maybe it will grow on you.";
    n.terminal = true;
  } else if (id == 50) {
    n.id = 50;
    n.text = "Which season do you like most?";
    n.options = {{"Spring", 51}, {"Fall", 52}};
  } else if (id == 51) {
    n.id = 51;
    n.text = "Spring flowers are beautiful.";
    n.terminal = true;
  } else if (id == 52) {
    n.id = 52;
    n.text = "I love the colors in fall.";
    n.terminal = true;
  } else if (id == 60) {
    n.id = 60;
    n.text = "You look tired. Did you work late?";
    n.options = {{"Yes", 61}, {"Not really", 62}};
  } else if (id == 61) {
    n.id = 61;
    n.text = "Remember to rest sometimes.";
    n.terminal = true;
  } else if (id == 62) {
    n.id = 62;
    n.text = "Then maybe a walk around town will help.";
    n.terminal = true;
  } else if (id == 70) {
    n.id = 70;
    n.text = "Do you play any games when you are free?";
    n.options = {{"Yes", 71}, {"No", 72}};
  } else if (id == 71) {
    n.id = 71;
    n.text = "Maybe we can play together one day.";
    n.terminal = true;
  } else if (id == 72) {
    n.id = 72;
    n.text = "You should try something fun sometime.";
    n.terminal = true;
  } else if (id == 80) {
    n.id = 80;
    n.text = "Do you like the sound of rain?";
    n.options = {{"Yes", 81}, {"No", 82}};
  } else if (id == 81) {
    n.id = 81;
    n.text = "Rain makes me feel calm.";
    n.terminal = true;
  } else if (id == 82) {
    n.id = 82;
    n.text = "I guess sunny days are better for you.";
    n.terminal = true;
  } else if (id == 90) {
    n.id = 90;
    n.text = "Have you made any new friends here?";
    n.options = {{"A few", 91}, {"Not yet", 92}};
  } else if (id == 91) {
    n.id = 91;
    n.text = "That is good. People here are nice.";
    n.terminal = true;
  } else if (id == 92) {
    n.id = 92;
    n.text = "Give it time. You will.";
    n.terminal = true;
  }
  return n;
}

}  // namespace Controllers

