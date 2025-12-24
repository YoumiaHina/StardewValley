#pragma once

#include "cocos2d.h"
#include <string>
#include <vector>
#include "Game/WorldState.h"

namespace Controllers {

// NPC 社交面板 UI：
// - 作用：展示 NPC 名称、好感度、关系状态与任务列表等社交信息。
// - 职责边界：只负责面板创建与展示刷新，不负责好感度计算、任务生成等业务逻辑。
// - 主要协作对象：由 UIController 调度 show/hide；数据由 NPC/任务系统提供。
class NpcSocialPanelUI {
 public:
  // 构造：记录所属场景，用于挂接面板节点。
  explicit NpcSocialPanelUI(cocos2d::Scene* scene) : scene_(scene) {}

  // 显示社交面板：传入 NPC 名称、好感度、恋爱解锁状态与任务列表。
  void show(const std::string& npc_name,
            int friendship,
            bool romance_unlocked,
            const std::vector<Game::NpcQuest>* quests);

  // 隐藏社交面板。
  void hide();
  // 当前社交面板是否可见。
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
