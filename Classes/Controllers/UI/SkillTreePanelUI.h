#pragma once

#include "cocos2d.h"
#include <vector>

namespace Controllers {

// SkillTreePanelUI：技能等级面板 UI。
// - 职责边界：只负责面板节点创建、内容刷新与显示隐藏；不负责技能经验的业务累积。
// - 协作对象：UIController 负责面板生命周期与统一调度；SkillTreeSystem 负责提供展示数据与加成描述。
class SkillTreePanelUI {
public:
    // 构造：绑定当前场景，用于创建并挂接面板节点。
    explicit SkillTreePanelUI(cocos2d::Scene* scene) : _scene(scene) {}

    // 构建面板节点（背景、标题与四条技能树信息行）。
    void buildSkillTreePanel();
    // 刷新面板内容（等级、经验、升下一级所需经验、可用点数与当前加成）。
    void refreshSkillTreePanel();
    // 显示/隐藏面板；show=true 时会确保构建并刷新。
    void toggleSkillTreePanel(bool show);
    // 面板是否处于可见状态。
    bool isVisible() const;

private:
    cocos2d::Scene* _scene = nullptr;
    cocos2d::Node* _panelNode = nullptr;
    cocos2d::Label* _titleLabel = nullptr;
    std::vector<cocos2d::Label*> _rows;
};

}

