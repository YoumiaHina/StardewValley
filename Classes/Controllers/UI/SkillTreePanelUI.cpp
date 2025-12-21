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

    _bgNode = DrawNode::create();
    _panelW = 520.f * SKILL_UI_SCALE;
    _panelH = 340.f * SKILL_UI_SCALE;
    Vec2 v[4] = {
        Vec2(-_panelW / 2, -_panelH / 2),
        Vec2(_panelW / 2, -_panelH / 2),
        Vec2(_panelW / 2, _panelH / 2),
        Vec2(-_panelW / 2, _panelH / 2)
    };
    _bgNode->drawSolidPoly(v, 4, Color4F(0.f, 0.f, 0.f, 0.85f));
    _panelNode->addChild(_bgNode);

    _titleLabel = createCnLabel("Skills (L)", 24 * SKILL_UI_SCALE);
    _titleLabel->setPosition(Vec2(0, _panelH / 2 - 26 * SKILL_UI_SCALE));
    _panelNode->addChild(_titleLabel);

    _rows.clear();
    float startY = _panelH / 2 - 70 * SKILL_UI_SCALE;
    float rowGap = 42 * SKILL_UI_SCALE;
    for (int i = 0; i < 6; ++i) {
        auto* lb = createCnLabel("", 18 * SKILL_UI_SCALE);
        lb->setAnchorPoint(Vec2(0.0f, 0.5f));
        lb->setPosition(Vec2(-_panelW / 2 + 22 * SKILL_UI_SCALE, startY - rowGap * i));
        _panelNode->addChild(lb);
        _rows.push_back(lb);
    }
}

void SkillTreePanelUI::refreshSkillTreePanel() {
    if (!_panelNode) return;
    auto& sys = Game::SkillTreeSystem::getInstance();
    auto visibleSize = Director::getInstance()->getVisibleSize();

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
        std::string bonus = sys.bonusDescription(t);

        std::string line = std::string(def.displayName())
            + " Lv" + std::to_string(lvl)
            + " | XP " + std::to_string(xp)
            + " | Next " + std::to_string(toNext)
            + " | " + bonus;
        _rows[i]->setString(line);
    }

    const float minW = 520.f * SKILL_UI_SCALE;
    const float minH = 340.f * SKILL_UI_SCALE;
    const float padX = 22.f * SKILL_UI_SCALE;
    const float topPad = 26.f * SKILL_UI_SCALE;
    const float bottomPad = 22.f * SKILL_UI_SCALE;
    const float headerGap = 18.f * SKILL_UI_SCALE;
    const float rowGap = 10.f * SKILL_UI_SCALE;

    float maxLineW = 0.0f;
    if (_titleLabel) {
        maxLineW = std::max(maxLineW, _titleLabel->getContentSize().width);
    }
    for (auto* lb : _rows) {
        if (!lb) continue;
        maxLineW = std::max(maxLineW, lb->getContentSize().width);
    }

    float maxAllowedW = std::max(minW, visibleSize.width * 0.95f);
    float maxAllowedH = std::max(minH, visibleSize.height * 0.95f);
    float desiredW = std::max(minW, std::min(maxAllowedW, maxLineW + padX * 2.0f));

    bool needWrap = (maxLineW + padX * 2.0f) > maxAllowedW;
    if (needWrap) {
        desiredW = maxAllowedW;
        float textW = std::max(1.0f, desiredW - padX * 2.0f);
        for (auto* lb : _rows) {
            if (!lb) continue;
            lb->setDimensions(textW, 0.0f);
            lb->setOverflow(Label::Overflow::RESIZE_HEIGHT);
        }
    } else {
        for (auto* lb : _rows) {
            if (!lb) continue;
            lb->setDimensions(0.0f, 0.0f);
        }
    }

    float titleH = _titleLabel ? _titleLabel->getContentSize().height : 0.0f;
    float totalRowsH = 0.0f;
    for (std::size_t i = 0; i < _rows.size(); ++i) {
        auto* lb = _rows[i];
        if (!lb) continue;
        totalRowsH += lb->getContentSize().height;
        if (i + 1 < _rows.size()) totalRowsH += rowGap;
    }
    float desiredH = std::max(minH, topPad + titleH + headerGap + totalRowsH + bottomPad);
    desiredH = std::min(desiredH, maxAllowedH);

    if (_bgNode && (desiredW != _panelW || desiredH != _panelH)) {
        _panelW = desiredW;
        _panelH = desiredH;
        _bgNode->clear();
        Vec2 v[4] = {
            Vec2(-_panelW / 2, -_panelH / 2),
            Vec2(_panelW / 2, -_panelH / 2),
            Vec2(_panelW / 2, _panelH / 2),
            Vec2(-_panelW / 2, _panelH / 2)
        };
        _bgNode->drawSolidPoly(v, 4, Color4F(0.f, 0.f, 0.f, 0.85f));
    }

    if (_titleLabel) {
        _titleLabel->setPosition(Vec2(0, _panelH / 2 - topPad - titleH * 0.5f));
    }

    float xLeft = -_panelW / 2 + padX;
    float y = _panelH / 2 - topPad - titleH - headerGap;
    for (auto* lb : _rows) {
        if (!lb) continue;
        float rh = lb->getContentSize().height;
        y -= rh * 0.5f;
        lb->setPosition(Vec2(xLeft, y));
        y -= rh * 0.5f;
        y -= rowGap;
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
