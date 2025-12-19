#include "Controllers/UI/DialogueUI.h"

using namespace cocos2d;
using namespace cocos2d::ui;

namespace Controllers {

namespace {

std::string npcDialogueBgPath(const std::string& npcName) {
  if (npcName == "Abigail") return "NPC/Abigail/Abigail-DialogBox.png";
  if (npcName == "Willy") return "NPC/Willy/Willy-DialoBox.png";
  if (npcName == "Robin") return "NPC/Robin/Robin-DialoBox.png";
  if (npcName == "Pierre") return "NPC/Pierre/Pierre-DialoBox.png";
  return "";
}

}  // namespace

void DialogueUI::show(const std::string& npc_name,
                      const std::string& text,
                      const std::vector<std::string>& options,
                      std::function<void(int)> on_option,
                      std::function<void()> on_advance) {
  on_option_ = std::move(on_option);
  on_advance_ = std::move(on_advance);
  auto visible = Director::getInstance()->getVisibleSize();
  auto origin = Director::getInstance()->getVisibleOrigin();
  float targetW = visible.width * 0.8f;
  float fallbackH = visible.height * 0.25f;
  if (!panel_) {
    panel_ = Node::create();
    if (scene_) scene_->addChild(panel_, 6);
    fallback_bg_ = DrawNode::create();
    if (fallback_bg_) panel_->addChild(fallback_bg_, -1);
    text_label_ = Label::createWithTTF("", "fonts/Marker Felt.ttf", 30);
    text_label_->setAnchorPoint(Vec2(0, 1));
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

  std::string nextBg = npcDialogueBgPath(npc_name);
  if (!nextBg.empty() && (!bg_ || bg_path_ != nextBg)) {
    if (bg_) {
      bg_->removeFromParent();
      bg_ = nullptr;
    }
    bg_ = Sprite::create(nextBg);
    bg_path_ = nextBg;
    if (bg_) {
      bg_->setAnchorPoint(Vec2(0.5f, 0.f));
      bg_->setPosition(Vec2(0, 0));
      panel_->addChild(bg_, -1);
    }
  }

  bool hasBg = (!nextBg.empty() && bg_ != nullptr);
  if (!hasBg && bg_) {
    bg_->setVisible(false);
  }

  float bgScale = 1.0f;
  float bgW = targetW;
  float bgH = fallbackH;
  if (hasBg) {
    auto s = bg_->getContentSize();
    if (s.width > 1e-3f) {
      bgScale = targetW / s.width;
      bgW = s.width * bgScale;
      bgH = s.height * bgScale;
    }
  }

  panel_->setPosition(Vec2(origin.x + visible.width * 0.5f,
                           origin.y + visible.height * 0.15f));
  panel_->setContentSize(Size(bgW, bgH));

  if (hasBg) {
    bg_->setScale(bgScale);
    bg_->setVisible(true);
  }

  if (fallback_bg_) {
    fallback_bg_->setVisible(!hasBg);
    fallback_bg_->clear();
    Vec2 v[4] = {Vec2(-bgW * 0.5f, 0), Vec2(bgW * 0.5f, 0),
                 Vec2(bgW * 0.5f, bgH), Vec2(-bgW * 0.5f, bgH)};
    fallback_bg_->drawSolidPoly(v, 4, Color4F(0.f, 0.f, 0.f, 0.7f));
  }

  float uiScale = std::max(0.85f, std::min(1.35f, bgScale));
  float padX = bgW * 0.06f;
  float padTop = bgH * 0.14f;
  float padBottom = bgH * 0.10f;

  if (text_label_) {
    text_label_->setScale(uiScale);
    float textTopY = bgH - padTop - 8.0f * uiScale;
    text_label_->setWidth(std::max(40.0f, (bgW - 2.0f * padX) / uiScale));
    text_label_->setPosition(Vec2(-bgW * 0.5f + padX, textTopY));
  }

  if (options.empty()) {
    for (auto* b : option_buttons_) {
      if (b && b->getParent()) b->removeFromParent();
    }
    option_buttons_.clear();
  } else {
    float gapX = 16.0f * uiScale;
    float btnH = 48.0f * uiScale;
    int count = static_cast<int>(options.size());
    float availableW = std::max(60.0f, bgW - 2.0f * padX);
    float btnW = (availableW - gapX * (count - 1)) / static_cast<float>(count);
    btnW = std::max(80.0f * uiScale, std::min(180.0f * uiScale, btnW));
    float totalW = btnW * count + gapX * (count - 1);
    float baseX = -totalW * 0.5f + btnW * 0.5f - 200.0f;
    float y = padBottom + btnH * 0.5f;

    while (static_cast<int>(option_buttons_.size()) > count) {
      auto* b = option_buttons_.back();
      if (b && b->getParent()) b->removeFromParent();
      option_buttons_.pop_back();
    }

    for (int i = 0; i < count; ++i) {
      Button* btn = nullptr;
      if (i < static_cast<int>(option_buttons_.size())) {
        btn = option_buttons_[i];
      }
      if (!btn) {
        btn = Button::create("NPC/Option-button.png",
                             "NPC/Option-button.png");
        if (btn) {
          btn->addClickEventListener([this](Ref* r) {
            auto* n = dynamic_cast<Node*>(r);
            int idx = n ? n->getTag() : 0;
            if (on_option_) on_option_(idx);
          });
          panel_->addChild(btn);
          option_buttons_.push_back(btn);
        }
      }
      if (!btn) continue;
      btn->setTag(i);
      btn->setTitleFontName("fonts/Marker Felt.ttf");
      btn->setTitleText(options[i]);
      btn->setTitleFontSize(25.0f * uiScale);
      btn->setTitleColor(Color3B(139, 69, 19));
      btn->setScale9Enabled(true);
      Size targetSize(btnW, btnH);
      auto titleNode = btn->getTitleRenderer();
      auto* titleLabel = dynamic_cast<Label*>(titleNode);
      if (titleLabel) {
        Size titleSize = titleLabel->getContentSize();
        float padBtnX = 16.0f * uiScale;
        float padBtnY = 8.0f * uiScale;
        float neededW = titleSize.width + padBtnX * 2.0f;
        float neededH = titleSize.height + padBtnY * 2.0f;
        if (neededW > targetSize.width) targetSize.width = neededW;
        if (neededH > targetSize.height) targetSize.height = neededH;
      }
      btn->setContentSize(targetSize);
      btn->setPosition(Vec2(baseX + i * (btnW + gapX), y));
    }
  }

  panel_->setVisible(true);
  text_label_->setString(text);
  text_label_->setTextColor(Color4B(139, 69, 19, 255));
}

void DialogueUI::hide() {
  if (panel_) panel_->setVisible(false);
}

bool DialogueUI::isVisible() const {
  return panel_ && panel_->isVisible();
}

}  // namespace Controllers
