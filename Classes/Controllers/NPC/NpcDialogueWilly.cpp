#include "Controllers/NPC/NpcDialogueManager.h"
#include <cstdlib>

namespace Controllers {

// Willy 对话脚本的起始节点：从一组候选中随机挑选一个。
int firstNodeForWilly() {
  static int variants[] = {101, 110, 120, 130, 140, 150, 160, 170, 180, 190};
  int idx = std::rand() % 10;
  return variants[idx];
}

// Willy 单个对话节点：按 id 返回对应的台词与选项。
DialogueNode nodeForWilly(int id) {
  DialogueNode n;
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
  return n;
}

}  // namespace Controllers

