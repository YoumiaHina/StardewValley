#pragma once

#include "cocos2d.h"
#include <string>

namespace Controllers {

class PromptUI {
public:
    PromptUI(cocos2d::Scene* scene, cocos2d::Node* worldNode)
      : _scene(scene), _worldNode(worldNode) {}

    void showDoorPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    void showChestPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    void showFishPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    void showNpcPrompt(bool visible, const cocos2d::Vec2& worldPos, const std::string& text);
    void popTextAt(const cocos2d::Vec2& worldPos, const std::string& text, const cocos2d::Color3B& color);

private:
    cocos2d::Scene* _scene = nullptr;
    cocos2d::Node* _worldNode = nullptr;
    cocos2d::Label* _doorPrompt = nullptr;
    cocos2d::Label* _chestPrompt = nullptr;
    cocos2d::Label* _fishPrompt = nullptr;
    cocos2d::Label* _npcPrompt = nullptr;
};

}
