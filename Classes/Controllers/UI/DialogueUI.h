#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <string>
#include <vector>
#include <functional>

namespace Controllers {

class DialogueUI {
 public:
  DialogueUI(cocos2d::Scene* scene)
      : scene_(scene) {}

  void show(const std::string& npc_name,
            const std::string& text,
            const std::vector<std::string>& options,
            std::function<void(int)> on_option,
            std::function<void()> on_advance);

  void hide();

  bool isVisible() const;

 private:
  cocos2d::Scene* scene_ = nullptr;
  cocos2d::Node* panel_ = nullptr;
  cocos2d::Label* name_label_ = nullptr;
  cocos2d::Label* text_label_ = nullptr;
  std::vector<cocos2d::ui::Button*> option_buttons_;
  std::function<void(int)> on_option_;
  std::function<void()> on_advance_;
};

}  // namespace Controllers

