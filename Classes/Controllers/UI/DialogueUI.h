#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <string>
#include <vector>
#include <functional>

namespace Controllers {

// 对话 UI：
// - 作用：显示 NPC 对话文本与选项按钮，并把选择结果/继续对话操作回调给上层。
// - 职责边界：只负责对话面板的创建、展示与输入响应，不承载剧情/对话树业务规则。
// - 主要协作对象：由 UIController 调度 show/hide；交互器/NPC 控制器提供文本与回调。
class DialogueUI {
 public:
  // 构造：记录所属场景，用于挂接对话面板节点。
  DialogueUI(cocos2d::Scene* scene)
      : scene_(scene) {}

  // 显示对话：设置 NPC 名称、文本、选项与回调（选择/继续）。
  void show(const std::string& npc_name,
            const std::string& text,
            const std::vector<std::string>& options,
            std::function<void(int)> on_option,
            std::function<void()> on_advance);

  // 隐藏对话面板。
  void hide();

  // 当前对话面板是否可见。
  bool isVisible() const;

 private:
  cocos2d::Scene* scene_ = nullptr;
  cocos2d::Node* panel_ = nullptr;
  cocos2d::DrawNode* fallback_bg_ = nullptr;
  cocos2d::Sprite* bg_ = nullptr;
  std::string bg_path_;
  cocos2d::Label* text_label_ = nullptr;
  std::vector<cocos2d::ui::Button*> option_buttons_;
  std::function<void(int)> on_option_;
  std::function<void()> on_advance_;
};

}  // namespace Controllers

