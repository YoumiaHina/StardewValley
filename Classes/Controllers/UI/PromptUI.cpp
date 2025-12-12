#include "Controllers/UI/PromptUI.h"

using namespace cocos2d;

namespace Controllers {

void PromptUI::showDoorPrompt(bool visible, const Vec2& worldPos, const std::string& text) {
    if (!_doorPrompt) {
        _doorPrompt = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
        _doorPrompt->setAnchorPoint(Vec2(0.5f, 0.5f));
        if (_scene) _scene->addChild(_doorPrompt, 3);
    }
    _doorPrompt->setString(text);
    _doorPrompt->setVisible(visible);
    if (visible) {
        Vec2 p = worldPos;
        if (_worldNode) p = _worldNode->convertToWorldSpace(worldPos);
        _doorPrompt->setPosition(p + Vec2(0, 26));
    }
}

void PromptUI::showChestPrompt(bool visible, const Vec2& worldPos, const std::string& text) {
    if (!_chestPrompt) {
        _chestPrompt = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
        _chestPrompt->setAnchorPoint(Vec2(0.5f, 0.5f));
        if (_scene) _scene->addChild(_chestPrompt, 3);
    }
    _chestPrompt->setString(text);
    _chestPrompt->setVisible(visible);
    if (visible) {
        Vec2 p = worldPos;
        if (_worldNode) p = _worldNode->convertToWorldSpace(worldPos);
        _chestPrompt->setPosition(p + Vec2(0, 26));
    }
}

void PromptUI::showFishPrompt(bool visible, const Vec2& worldPos, const std::string& text) {
    if (!_fishPrompt) {
        _fishPrompt = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
        _fishPrompt->setAnchorPoint(Vec2(0.5f, 0.5f));
        if (_scene) _scene->addChild(_fishPrompt, 3);
    }
    _fishPrompt->setString(text);
    _fishPrompt->setVisible(visible);
    if (visible) {
        Vec2 p = worldPos;
        if (_worldNode) p = _worldNode->convertToWorldSpace(worldPos);
        _fishPrompt->setPosition(p + Vec2(0, 26));
    }
}

void PromptUI::popTextAt(const Vec2& worldPos, const std::string& text, const Color3B& color) {
    auto label = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 16);
    label->setColor(color);
    Vec2 p = worldPos;
    if (_worldNode) p = _worldNode->convertToWorldSpace(p);
    label->setPosition(p);
    if (_scene) _scene->addChild(label, 4);
    auto up = MoveBy::create(0.8f, Vec2(0, 24));
    auto fade = FadeOut::create(0.8f);
    auto rm = RemoveSelf::create();
    label->runAction(Sequence::create(Spawn::create(up, fade, nullptr), rm, nullptr));
}

}
