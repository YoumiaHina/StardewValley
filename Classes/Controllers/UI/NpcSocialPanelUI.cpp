#include "Controllers/UI/NpcSocialPanelUI.h"
#include "ui/CocosGUI.h"

using namespace cocos2d;
using namespace cocos2d::ui;

namespace Controllers {

void NpcSocialPanelUI::show(const std::string& npc_name,
                            int friendship,
                            bool romance_unlocked,
                            const std::vector<Game::NpcQuest>* quests) {
  if (!layer_) {
    auto visible = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    layer_ = LayerColor::create(Color4B(0, 0, 0, 120));
    if (scene_) scene_->addChild(layer_, 7);
    panel_ = Node::create();
    float w = visible.width * 0.6f;
    float h = visible.height * 0.5f;
    panel_->setPosition(
        Vec2(origin.x + visible.width * 0.5f,
             origin.y + visible.height * 0.5f));
    layer_->addChild(panel_);
    bg_ = DrawNode::create();
    panel_->addChild(bg_);
    portrait_ = Sprite::create();
    if (portrait_) {
      portrait_->setAnchorPoint(Vec2(0.5f, 0.5f));
      portrait_->setPosition(Vec2(-w * 0.5f + 80.f, 0));
      panel_->addChild(portrait_);
    }
    name_label_ = Label::createWithTTF("", "fonts/Marker Felt.ttf", 22);
    name_label_->setPosition(Vec2(0, h * 0.5f - 28.f));
    panel_->addChild(name_label_);
    friendship_label_ =
        Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
    friendship_label_->setAnchorPoint(Vec2(0, 0.5f));
    friendship_label_->setPosition(
        Vec2(-w * 0.5f + 160.f, h * 0.2f));
    panel_->addChild(friendship_label_);
    hearts_node_ = Node::create();
    hearts_node_->setPosition(
        Vec2(-w * 0.5f + 160.f, h * 0.1f));
    panel_->addChild(hearts_node_);
    relation_label_ =
        Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
    relation_label_->setAnchorPoint(Vec2(0, 0.5f));
    relation_label_->setPosition(
        Vec2(-w * 0.5f + 160.f, 0));
    panel_->addChild(relation_label_);
    quests_node_ = Node::create();
    quests_node_->setPosition(Vec2(-w * 0.5f + 160.f, -h * 0.2f));
    panel_->addChild(quests_node_);
    auto closeBtn = Button::create("CloseNormal.png", "CloseSelected.png");
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(18);
    closeBtn->setScale9Enabled(true);
    closeBtn->setContentSize(Size(32.f, 32.f));
    closeBtn->setPosition(Vec2(w * 0.5f - 24.f, h * 0.5f - 24.f));
    closeBtn->addClickEventListener([this](Ref*) { hide(); });
    panel_->addChild(closeBtn);
    auto esc = EventListenerKeyboard::create();
    esc->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) {
      if (code == EventKeyboard::KeyCode::KEY_ESCAPE) hide();
    };
    layer_->getEventDispatcher()->addEventListenerWithSceneGraphPriority(
        esc, layer_);
  }
  if (!layer_) return;
  layer_->setVisible(true);
  if (panel_) panel_->setVisible(true);
  int maxFriend = 250;
  int clamped =
      friendship < 0 ? 0 : (friendship > maxFriend ? maxFriend : friendship);
  if (bg_) {
    bg_->clear();
    auto visible = Director::getInstance()->getVisibleSize();
    float w = visible.width * 0.6f;
    float h = visible.height * 0.5f;
    Vec2 v[4] = {Vec2(-w * 0.5f, -h * 0.5f), Vec2(w * 0.5f, -h * 0.5f),
                 Vec2(w * 0.5f, h * 0.5f), Vec2(-w * 0.5f, h * 0.5f)};
    float t = static_cast<float>(clamped) / 250.0f;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    Color4F cold(0.1f, 0.1f, 0.2f, 0.85f);
    Color4F warm(0.4f, 0.25f, 0.05f, 0.9f);
    float r = cold.r + (warm.r - cold.r) * t;
    float g = cold.g + (warm.g - cold.g) * t;
    float b = cold.b + (warm.b - cold.b) * t;
    float a = cold.a + (warm.a - cold.a) * t;
    bg_->drawSolidPoly(v, 4, Color4F(r, g, b, a));
  }
  if (name_label_) name_label_->setString(npc_name);
  if (friendship_label_) {
    friendship_label_->setString(
        StringUtils::format("Friendship: %d / %d", clamped, maxFriend));
  }
  if (hearts_node_) {
    hearts_node_->removeAllChildren();
    int filled = clamped / 25;
    if (filled > 10) filled = 10;
    int total = 10;
    float gap = 20.f;
    float startX = 0.f;
    for (int i = 0; i < total; ++i) {
      auto s = Sprite::create("NPC/Friendship.png");
      if (!s || !s->getTexture()) continue;
      s->setPosition(Vec2(startX + i * gap, 0));
      if (i < filled) {
        s->setColor(Color3B(255, 160, 160));
        s->setOpacity(255);
      } else {
        s->setColor(Color3B(120, 120, 130));
        s->setOpacity(180);
      }
      s->setScale(1.4f);
      hearts_node_->addChild(s);
    }
  }
  if (relation_label_) {
    std::string rel;
    if (clamped < 25) rel = "Stranger";
    else if (clamped < 100) rel = "Friend";
    else if (clamped < 180) rel = "Close Friend";
    else rel = romance_unlocked ? "Romantic" : "Romantic (locked)";
    relation_label_->setString("Relation: " + rel);
    cocos2d::Color3B color;
    if (clamped < 25) {
      color = Color3B(150, 150, 150);
    } else if (clamped < 100) {
      color = Color3B(200, 200, 230);
    } else if (clamped < 180) {
      color = Color3B(255, 230, 190);
    } else if (romance_unlocked) {
      color = Color3B(255, 185, 210);
    } else {
      color = Color3B(230, 210, 210);
    }
    relation_label_->setColor(color);
  }
  if (portrait_) {
    std::string tex;
    if (npc_name == "Abigail") tex = "NPC/Abigail-standing.png";
    else if (npc_name == "Willy") tex = "NPC/Willy-standing.png";
    if (!tex.empty()) {
      portrait_->setTexture(tex);
      auto size = portrait_->getContentSize();
      if (size.width > 0 && size.height > 0) {
        auto visible = Director::getInstance()->getVisibleSize();
        float w = visible.width * 0.6f;
        float h = visible.height * 0.5f;
        float maxW = w * 0.25f;
        float maxH = h * 0.7f;
        float sx = maxW / size.width;
        float sy = maxH / size.height;
        float s = std::min(sx, sy);
        if (s <= 0.f) s = 1.f;
        portrait_->setScale(s);
      }
    }
  }
  if (quests_node_) {
    quests_node_->removeAllChildren();
    float y = 0;
    auto title = Label::createWithTTF("Current Quests", "fonts/Marker Felt.ttf", 20);
    title->setAnchorPoint(Vec2(0, 0.5f));
    title->setPosition(Vec2(0, y));
    quests_node_->addChild(title);
    y -= 28.f;
    if (!quests || quests->empty()) {
      auto empty =
          Label::createWithTTF("No quests available", "fonts/Marker Felt.ttf", 18);
      empty->setAnchorPoint(Vec2(0, 0.5f));
      empty->setPosition(Vec2(0, y));
      quests_node_->addChild(empty);
    } else {
      for (const auto& q : *quests) {
        auto line = Label::createWithTTF(
            q.title + " - " + q.description,
            "fonts/Marker Felt.ttf", 18);
        line->setAnchorPoint(Vec2(0, 0.5f));
        line->setPosition(Vec2(0, y));
        quests_node_->addChild(line);
        y -= 22.f;
      }
    }
  }
}

void NpcSocialPanelUI::hide() {
  if (layer_) layer_->setVisible(false);
  if (panel_) panel_->setVisible(false);
}

bool NpcSocialPanelUI::isVisible() const {
  return layer_ && layer_->isVisible();
}

}  // namespace Controllers
