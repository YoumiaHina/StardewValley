#include "Controllers/NPC/NpcDialogueManager.h"
#include "Controllers/UI/UIController.h"
#include <cstdlib>

using namespace cocos2d;

namespace Controllers {

int NpcDialogueManager::firstNodeFor(int npcKey) const {
  if (npcKey == 1) {
    static int variants[] = {1, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    int idx = std::rand() % 10;
    return variants[idx];
  }
  if (npcKey == 2) {
    static int variants[] = {101, 110, 120, 130, 140, 150, 160, 170, 180, 190};
    int idx = std::rand() % 10;
    return variants[idx];
  }
  return 0;
}

DialogueNode NpcDialogueManager::nodeFor(int npcKey, int id) const {
  DialogueNode n;
  if (npcKey == 1) {
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
  } else if (npcKey == 2) {
    if (id == 101) {
      n.id = 101;
      n.text = "Name is Willy. Do you like fishing?";
      n.options = {{"I love it", 102}, {"Not really", 103}};
    } else if (id == 102) {
      n.id = 102;
      n.text = "Then you should visit the beach more often.";
      n.terminal = true;
    } else if (id == 103) {
      n.id = 103;
      n.text = "Maybe one day you will change your mind.";
      n.terminal = true;
    } else if (id == 110) {
      n.id = 110;
      n.text = "The sea is calm today.";
      n.options = {{"Peaceful", 111}, {"Boring", 112}};
    } else if (id == 111) {
      n.id = 111;
      n.text = "A perfect time to cast a line.";
      n.terminal = true;
    } else if (id == 112) {
      n.id = 112;
      n.text = "You prefer storms, eh?";
      n.terminal = true;
    } else if (id == 120) {
      n.id = 120;
      n.text = "Do you need bait?";
      n.options = {{"Yes, please", 121}, {"No, thanks", 122}};
    } else if (id == 121) {
      n.id = 121;
      n.text = "Visit my shop later.";
      n.terminal = true;
    } else if (id == 122) {
      n.id = 122;
      n.text = "Alright. Tight lines!";
      n.terminal = true;
    } else if (id == 130) {
      n.id = 130;
      n.text = "Have you caught any rare fish?";
      n.options = {{"Yes", 131}, {"Not yet", 132}};
    } else if (id == 131) {
      n.id = 131;
      n.text = "You must be proud of that.";
      n.terminal = true;
    } else if (id == 132) {
      n.id = 132;
      n.text = "Keep trying. The sea is full of surprises.";
      n.terminal = true;
    } else if (id == 140) {
      n.id = 140;
      n.text = "Do you prefer river or sea fishing?";
      n.options = {{"River", 141}, {"Sea", 142}};
    } else if (id == 141) {
      n.id = 141;
      n.text = "River fish can be tricky.";
      n.terminal = true;
    } else if (id == 142) {
      n.id = 142;
      n.text = "Waves and salt, my favorites too.";
      n.terminal = true;
    } else if (id == 150) {
      n.id = 150;
      n.text = "Ever been out on a boat?";
      n.options = {{"Yes", 151}, {"No", 152}};
    } else if (id == 151) {
      n.id = 151;
      n.text = "Nothing like the wind on the deck.";
      n.terminal = true;
    } else if (id == 152) {
      n.id = 152;
      n.text = "Maybe one day I can show you.";
      n.terminal = true;
    } else if (id == 160) {
      n.id = 160;
      n.text = "Do you wake up early?";
      n.options = {{"Always", 161}, {"Not really", 162}};
    } else if (id == 161) {
      n.id = 161;
      n.text = "Good. Fish bite best in the morning.";
      n.terminal = true;
    } else if (id == 162) {
      n.id = 162;
      n.text = "You might miss the best catch.";
      n.terminal = true;
    } else if (id == 170) {
      n.id = 170;
      n.text = "Do storms scare you?";
      n.options = {{"A bit", 171}, {"Not at all", 172}};
    } else if (id == 171) {
      n.id = 171;
      n.text = "Stay inside then, let me handle the sea.";
      n.terminal = true;
    } else if (id == 172) {
      n.id = 172;
      n.text = "You are braver than many sailors.";
      n.terminal = true;
    } else if (id == 180) {
      n.id = 180;
      n.text = "Do you sell your fish often?";
      n.options = {{"Yes", 181}, {"I keep them", 182}};
    } else if (id == 181) {
      n.id = 181;
      n.text = "Gold from the sea is still gold.";
      n.terminal = true;
    } else if (id == 182) {
      n.id = 182;
      n.text = "Fresh dinner is never a bad idea.";
      n.terminal = true;
    } else if (id == 190) {
      n.id = 190;
      n.text = "Ever found anything strange while fishing?";
      n.options = {{"Yes", 191}, {"No", 192}};
    } else if (id == 191) {
      n.id = 191;
      n.text = "The sea keeps secrets.";
      n.terminal = true;
    } else if (id == 192) {
      n.id = 192;
      n.text = "Keep your eyes open next time.";
      n.terminal = true;
    }
  }
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

}  // namespace Controllers
