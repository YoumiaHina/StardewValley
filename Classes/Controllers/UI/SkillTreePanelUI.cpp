#include "Controllers/UI/SkillTreePanelUI.h"

#include "Game/SkillTree/SkillTreeSystem.h"

using namespace cocos2d;

namespace Controllers {

namespace {
    const float SKILL_UI_SCALE = 1.2f;

    cocos2d::Label* createCnLabel(const std::string& text, float fontSize) {
        const char* sysTtfCandidates[] = {
            "C:/Windows/Fonts/msyh.ttc",
            "C:/Windows/Fonts/msyhl.ttc",
            "C:/Windows/Fonts/simhei.ttf",
            "C:/Windows/Fonts/simsun.ttc",
            "C:/Windows/Fonts/msjh.ttc"
        };
        for (const char* path : sysTtfCandidates) {
            if (cocos2d::FileUtils::getInstance()->isFileExist(path)) {
                if (auto* lb = cocos2d::Label::createWithTTF(text, path, fontSize)) return lb;
            }
        }

        const char* sysFontCandidates[] = {
            "Microsoft YaHei",
            "Microsoft YaHei UI",
            "SimHei",
            "SimSun",
            "NSimSun"
        };
        for (const char* name : sysFontCandidates) {
            if (auto* lb = cocos2d::Label::createWithSystemFont(text, name, fontSize)) return lb;
        }

        if (auto* lb = cocos2d::Label::createWithTTF(text, "fonts/arial.ttf", fontSize)) return lb;
        return cocos2d::Label::createWithTTF(text, "fonts/Marker Felt.ttf", fontSize);
    }
}

void SkillTreePanelUI::buildSkillTreePanel() {
    if (_panelNode) return;
    _panelNode = Node::create();
    if (_scene) _scene->addChild(_panelNode, 6);
    _panelNode->setVisible(false);
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _panelNode->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));

    auto bg = DrawNode::create();
    float w = 520.f * SKILL_UI_SCALE;
    float h = 340.f * SKILL_UI_SCALE;
    Vec2 v[4] = { Vec2(-w / 2, -h / 2), Vec2(w / 2, -h / 2), Vec2(w / 2, h / 2), Vec2(-w / 2, h / 2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f, 0.f, 0.f, 0.85f));
    _panelNode->addChild(bg);

    _titleLabel = createCnLabel("Skills (L)  1-6 Unlock", 24 * SKILL_UI_SCALE);
    _titleLabel->setPosition(Vec2(0, h / 2 - 26 * SKILL_UI_SCALE));
    _panelNode->addChild(_titleLabel);

    _rows.clear();
    float startY = h / 2 - 70 * SKILL_UI_SCALE;
    float rowGap = 42 * SKILL_UI_SCALE;
    for (int i = 0; i < 6; ++i) {
        auto* lb = createCnLabel("", 18 * SKILL_UI_SCALE);
        lb->setAnchorPoint(Vec2(0.0f, 0.5f));
        lb->setPosition(Vec2(-w / 2 + 22 * SKILL_UI_SCALE, startY - rowGap * i));
        _panelNode->addChild(lb);
        _rows.push_back(lb);
    }
}

void SkillTreePanelUI::refreshSkillTreePanel() {
    if (!_panelNode) return;
    auto& sys = Game::SkillTreeSystem::getInstance();

    const Game::SkillTreeType types[6] = {
        Game::SkillTreeType::Farming,
        Game::SkillTreeType::AnimalHusbandry,
        Game::SkillTreeType::Forestry,
        Game::SkillTreeType::Fishing,
        Game::SkillTreeType::Mining,
        Game::SkillTreeType::Combat
    };

    for (int i = 0; i < 6 && i < static_cast<int>(_rows.size()); ++i) {
        auto t = types[i];
        const auto& def = sys.definition(t);
        int lvl = sys.level(t);
        int xp = sys.totalXp(t);
        int toNext = sys.xpToNextLevel(t);
        int pts = sys.unspentPoints(t);
        std::string bonus = sys.bonusDescription(t);

        std::string line = std::string(def.displayName())
            + " Lv" + std::to_string(lvl)
            + " | XP " + std::to_string(xp)
            + " | Next " + std::to_string(toNext)
            + " | Points " + std::to_string(pts)
            + " | " + bonus;
        _rows[i]->setString(line);
    }
}

void SkillTreePanelUI::toggleSkillTreePanel(bool show) {
    if (show) {
        buildSkillTreePanel();
        refreshSkillTreePanel();
        if (_panelNode) _panelNode->setVisible(true);
    } else {
        if (_panelNode) _panelNode->setVisible(false);
    }
}

bool SkillTreePanelUI::isVisible() const {
    return _panelNode && _panelNode->isVisible();
}

}
