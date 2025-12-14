#pragma once

#include "cocos2d.h"
#include <string>
#include <vector>
#include "Game/WorldState.h"

namespace Controllers {

class NpcSocialPanelUI {
 public:
  explicit NpcSocialPanelUI(cocos2d::Scene* scene) : scene_(scene) {}

  void show(const std::string& npc_name,
            int friendship,
            bool romance_unlocked,
            const std::vector<Game::NpcQuest>* quests);

  void hide();
  bool isVisible() const;

 private:
  cocos2d::Scene* scene_ = nullptr;
  cocos2d::LayerColor* layer_ = nullptr;
  cocos2d::Node* panel_ = nullptr;
  cocos2d::DrawNode* bg_ = nullptr;
  cocos2d::Label* name_label_ = nullptr;
  cocos2d::Label* friendship_label_ = nullptr;
  cocos2d::Node* hearts_node_ = nullptr;
  cocos2d::Label* relation_label_ = nullptr;
  cocos2d::Node* quests_node_ = nullptr;
  cocos2d::Sprite* portrait_ = nullptr;
};

}  // namespace Controllers
