#pragma once

#include "cocos2d.h"
#include <string>

namespace Controllers {

// 提示与飘字 UI：
// - 作用：在玩家附近显示门/箱子/钓鱼/NPC 等交互提示，并在世界坐标弹出短文本。
// - 职责边界：只负责提示节点创建与位置同步，不负责判定何时显示与文本内容生成。
// - 主要协作对象：UIController 负责调度显示/隐藏；交互/系统模块提供 worldPos 与文本。
class PromptUI {
public:
    // 构造：记录所属场景与世界节点，用于把提示挂到正确的坐标系。
    PromptUI(cocos2d::Scene* scene, cocos2d::Node* worldNode)
      : _scene(scene), _worldNode(worldNode) {}

    // 显示/隐藏门口提示，并同步到指定世界坐标。
    void showDoorPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    // 显示/隐藏箱子提示，并同步到指定世界坐标。
    void showChestPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    // 显示/隐藏钓鱼提示，并同步到指定世界坐标。
    void showFishPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    // 显示/隐藏 NPC 交互提示，并同步到指定世界坐标。
    void showNpcPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    // 在指定世界坐标弹出短文本（用于提示获得物品等）。
    void popTextAt(const cocos2d::Vec2& worldPos, const std::string& text, const cocos2d::Color3B& color);
    // 在指定世界坐标弹出好感度文本（通常与 NPC 交互相关）。
    void popFriendshipTextAt(const cocos2d::Vec2& worldPos, const std::string& text, const cocos2d::Color3B& color);
    // 在屏幕中心弹出大字号文本（用于事件/警告等提示）。
    void popCenterBigText(const std::string& text, const cocos2d::Color3B& color);

private:
    cocos2d::Scene* _scene = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    cocos2d::Label* _doorPrompt = nullptr;
    cocos2d::Label* _chestPrompt = nullptr;
    cocos2d::Label* _fishPrompt = nullptr;
    cocos2d::Label* _npcPrompt = nullptr;
};

}
