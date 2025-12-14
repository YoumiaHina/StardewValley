#include "Controllers/UI/DialogueUI.h"

using namespace cocos2d;
using namespace cocos2d::ui;

namespace Controllers {

void DialogueUI::show(const std::string& npc_name,
                      const std::string& text,
                      const std::vector<std::string>& options,
                      std::function<void(int)> on_option,
                      std::function<void()> on_advance) {
  on_option_ = std::move(on_option);
  on_advance_ = std::move(on_advance);
  if (!panel_) {
    panel_ = Node::create();
    if (scene_) scene_->addChild(panel_, 6);
    auto visible = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    panel_->setPosition(Vec2(origin.x + visible.width * 0.5f,
                             origin.y + visible.height * 0.15f));
    auto bg = DrawNode::create();
    float w = visible.width * 0.8f;
    float h = visible.height * 0.25f;
    Vec2 v[4] = {Vec2(-w * 0.5f, 0), Vec2(w * 0.5f, 0),
                 Vec2(w * 0.5f, h), Vec2(-w * 0.5f, h)};
    bg->drawSolidPoly(v, 4, Color4F(0.f, 0.f, 0.f, 0.7f));
    panel_->addChild(bg);
    name_label_ = Label::createWithTTF("", "fonts/Marker Felt.ttf", 20);
    name_label_->setAnchorPoint(Vec2(0, 0.5f));
    name_label_->setPosition(Vec2(-w * 0.5f + 20.f, h - 24.f));
    panel_->addChild(name_label_);
    text_label_ = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
    text_label_->setAnchorPoint(Vec2(0, 1));
    text_label_->setWidth(w - 40.f);
    text_label_->setPosition(Vec2(-w * 0.5f + 20.f, h - 60.f));
    panel_->addChild(text_label_);
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this](Touch* t, Event*) {
      if (!panel_ || !on_advance_) return false;
      Vec2 local = panel_->convertToNodeSpace(t->getLocation());
      auto size = panel_->getContentSize();
      Rect r(-size.width, 0, size.width * 2.f, size.height);
      if (!r.containsPoint(local)) return false;
      if (option_buttons_.empty()) on_advance_();
      return true;
    };
    panel_->getEventDispatcher()->addEventListenerWithSceneGraphPriority(
        listener, panel_);
  }
  if (!panel_) return;
  panel_->setVisible(true);
  name_label_->setString(npc_name);
  text_label_->setString(text);
  for (auto* b : option_buttons_) {
    if (b && b->getParent()) b->removeFromParent();
  }
  option_buttons_.clear();
  if (!options.empty()) {
    float startY = 20.f;
    float gapX = 120.f;
    int count = static_cast<int>(options.size());
    float baseX = -gapX * (count - 1) * 0.5f;
    for (int i = 0; i < count; ++i) {
      auto btn = Button::create("CloseNormal.png", "CloseSelected.png");
      btn->setTitleText(options[i]);
      btn->setTitleFontSize(18);
      btn->setScale9Enabled(true);
      btn->setContentSize(Size(110.f, 32.f));
      btn->setPosition(Vec2(baseX + i * gapX, startY));
      int idx = i;
      btn->addClickEventListener([this, idx](Ref*) {
        if (on_option_) on_option_(idx);
      });
      panel_->addChild(btn);
      option_buttons_.push_back(btn);
    }
  }
}

void DialogueUI::hide() {
  if (panel_) panel_->setVisible(false);
}

bool DialogueUI::isVisible() const {
  return panel_ && panel_->isVisible();
}

}  // namespace Controllers

