#include "Controllers/UI/CraftPanelUI.h"
#include "ui/CocosGUI.h"
#include "Game/Item.h"
#include <algorithm>

using namespace cocos2d;

namespace Controllers {

namespace {
    const float CRAFT_UI_SCALE = 1.3f;

    bool hitLabel(Label* label, Touch* t, Event* e) {
        if (!label || !t || !e) return false;
        auto target = static_cast<Label*>(e->getCurrentTarget());
        Vec2 p = target->convertToNodeSpace(t->getLocation());
        Size s = target->getContentSize();
        Rect r(0, 0, s.width, s.height);
        return r.containsPoint(p);
    }
}

void CraftPanelUI::buildCraftPanel() {
    if (_panelNode) return;
    _panelNode = Node::create();
    if (_scene) _scene->addChild(_panelNode, 5);
    _panelNode->setVisible(false);
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _panelNode->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));

    auto bg = DrawNode::create();
    float w = 620.f * CRAFT_UI_SCALE;
    float h = 380.f * CRAFT_UI_SCALE;
    Vec2 v[4] = { Vec2(-w/2, -h/2), Vec2(w/2, -h/2), Vec2(w/2, h/2), Vec2(-w/2, h/2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f, 0.f, 0.f, 0.85f));
    _panelNode->addChild(bg);

    _titleLabel = Label::createWithTTF("", "fonts/arial.ttf", 24 * CRAFT_UI_SCALE);
    _titleLabel->setPosition(Vec2(0, h/2 - 26 * CRAFT_UI_SCALE));
    _panelNode->addChild(_titleLabel);
    updateTitle();

    auto closeBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(18 * CRAFT_UI_SCALE);
    closeBtn->setScale9Enabled(true);
    closeBtn->setContentSize(Size(36 * CRAFT_UI_SCALE, 36 * CRAFT_UI_SCALE));
    closeBtn->setPosition(Vec2(w/2 - 20 * CRAFT_UI_SCALE, h/2 - 20 * CRAFT_UI_SCALE));
    closeBtn->addClickEventListener([this](Ref*) { toggleCraftPanel(false); });
    _panelNode->addChild(closeBtn);

    _tabsNode = Node::create();
    _panelNode->addChild(_tabsNode);
    float tabY = h/2 + 10 * CRAFT_UI_SCALE;
    float tabX1 = -90 * CRAFT_UI_SCALE;
    float tabX2 = 90 * CRAFT_UI_SCALE;
    float tabFont = 18 * CRAFT_UI_SCALE;

    _tabPlaceable = Label::createWithTTF("Placeable", "fonts/arial.ttf", tabFont);
    _tabFood = Label::createWithTTF("Food", "fonts/arial.ttf", tabFont);

    if (_tabPlaceable) { _tabPlaceable->setPosition(Vec2(tabX1, tabY)); _tabsNode->addChild(_tabPlaceable); }
    if (_tabFood) { _tabFood->setPosition(Vec2(tabX2, tabY)); _tabsNode->addChild(_tabFood); }

    auto bindTab = [this](Label* tab, Game::RecipeCategory cat) {
        if (!tab) return;
        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);
        listener->onTouchBegan = [tab](Touch* t, Event* e) {
            if (hitLabel(tab, t, e)) {
                tab->setScale(0.9f);
                return true;
            }
            return false;
        };
        listener->onTouchEnded = [this, tab, cat](Touch*, Event*) {
            tab->setScale(1.0f);
            setCategory(cat);
        };
        _tabsNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, tab);
    };

    bindTab(_tabPlaceable, Game::RecipeCategory::Placeable);
    bindTab(_tabFood, Game::RecipeCategory::Food);
    updateTabsVisual();

    _listNode = Node::create();
    _panelNode->addChild(_listNode);

    _craftingController = std::make_unique<Controllers::CraftingController>(_inventory);

    auto prevBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    prevBtn->setTitleText("<");
    prevBtn->setTitleFontSize(18 * CRAFT_UI_SCALE);
    prevBtn->setScale9Enabled(true);
    prevBtn->setContentSize(Size(36 * CRAFT_UI_SCALE, 36 * CRAFT_UI_SCALE));
    prevBtn->setPosition(Vec2(-w/2 + 30 * CRAFT_UI_SCALE, -h/2 + 30 * CRAFT_UI_SCALE));
    prevBtn->addClickEventListener([this](Ref*) {
        if (_pageIndex > 0) {
            _pageIndex--;
        }
        refreshCraftPanel();
    });
    _panelNode->addChild(prevBtn);

    auto nextBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    nextBtn->setTitleText(">");
    nextBtn->setTitleFontSize(18 * CRAFT_UI_SCALE);
    nextBtn->setScale9Enabled(true);
    nextBtn->setContentSize(Size(36 * CRAFT_UI_SCALE, 36 * CRAFT_UI_SCALE));
    nextBtn->setPosition(Vec2(w/2 - 30 * CRAFT_UI_SCALE, -h/2 + 30 * CRAFT_UI_SCALE));
    nextBtn->addClickEventListener([this](Ref*) {
        if (_recipes.empty()) refreshCraftPanel();
        int total = static_cast<int>(_recipes.size());
        int maxPage = std::max(1, (total + _pageSize - 1) / _pageSize);
        if (_pageIndex < maxPage - 1) {
            _pageIndex++;
        }
        refreshCraftPanel();
    });
    _panelNode->addChild(nextBtn);
}

void CraftPanelUI::refreshCraftPanel() {
    if (!_listNode || !_craftingController) return;
    _listNode->removeAllChildren();
    if (_recipes.empty()) {
        rebuildRecipes();
    }

    float startY = 85.0f * CRAFT_UI_SCALE;
    float gapY = 48.0f * CRAFT_UI_SCALE;
    int total = static_cast<int>(_recipes.size());
    int startIdx = std::max(0, _pageIndex * _pageSize);
    int endIdx = std::min(total, startIdx + _pageSize);
    if (startIdx >= total && total > 0) {
        _pageIndex = std::max(0, (total - 1) / _pageSize);
        startIdx = _pageIndex * _pageSize;
        endIdx = std::min(total, startIdx + _pageSize);
    }

    auto pageLabel = Label::createWithTTF(
        StringUtils::format("Page %d/%d", _pageIndex + 1, std::max(1, (total + _pageSize - 1) / _pageSize)),
        "fonts/arial.ttf",
        16 * CRAFT_UI_SCALE);
    pageLabel->setPosition(Vec2(0, -135 * CRAFT_UI_SCALE));
    _listNode->addChild(pageLabel);

    for (int row = 0, i = startIdx; i < endIdx; ++i, ++row) {
        auto recipe = _recipes[static_cast<std::size_t>(i)];
        if (!recipe) continue;
        float y = startY - row * gapY;
        Game::ItemType outType = recipe->outputType();

        std::string iconPath = Game::itemIconPath(outType);
        if (!iconPath.empty()) {
            auto icon = Sprite::create();
            icon->setTexture(iconPath);
            if (icon->getTexture()) {
                icon->getTexture()->setAliasTexParameters();
                auto cs = icon->getContentSize();
                float targetW = 28.0f * CRAFT_UI_SCALE;
                float targetH = 28.0f * CRAFT_UI_SCALE;
                float sx = (cs.width > 0) ? (targetW / cs.width) : 1.0f;
                float sy = (cs.height > 0) ? (targetH / cs.height) : 1.0f;
                icon->setScale(std::min(sx, sy));
                icon->setPosition(Vec2(-210 * CRAFT_UI_SCALE, y));
                _listNode->addChild(icon);
            }
        }

        std::string name = recipe->displayName();
        auto nameLabel = Label::createWithTTF(name, "fonts/arial.ttf", 20 * CRAFT_UI_SCALE);
        nameLabel->setAnchorPoint(Vec2(0, 0.5f));
        nameLabel->setPosition(Vec2(-185 * CRAFT_UI_SCALE, y + 10 * CRAFT_UI_SCALE));
        _listNode->addChild(nameLabel);

        std::string ingLine = formatIngredientsLine(*recipe);
        auto ingLabel = Label::createWithTTF(ingLine, "fonts/arial.ttf", 16 * CRAFT_UI_SCALE);
        ingLabel->setAnchorPoint(Vec2(0, 0.5f));
        ingLabel->setPosition(Vec2(-185 * CRAFT_UI_SCALE, y - 12 * CRAFT_UI_SCALE));
        ingLabel->setColor(Color3B(180, 180, 180));
        _listNode->addChild(ingLabel);

        bool can = _inventory && _craftingController->canCraft(*recipe);
        auto craftLabel = Label::createWithTTF("[Craft]", "fonts/arial.ttf", 20 * CRAFT_UI_SCALE);
        craftLabel->setPosition(Vec2(190 * CRAFT_UI_SCALE, y));
        craftLabel->setColor(can ? Color3B::GREEN : Color3B(120, 120, 120));
        _listNode->addChild(craftLabel);

        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);
        listener->onTouchBegan = [craftLabel, can](Touch* t, Event* e) {
            if (!can) return false;
            if (hitLabel(craftLabel, t, e)) {
                craftLabel->setScale(0.9f);
                return true;
            }
            return false;
        };
        listener->onTouchEnded = [this, recipe, craftLabel](Touch*, Event*) {
            craftLabel->setScale(1.0f);
            bool ok = false;
            if (recipe && _craftingController) {
                ok = _craftingController->craft(*recipe);
            }
            refreshCraftPanel();
            if (onCrafted) {
                onCrafted(ok);
            }
        };
        _listNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, craftLabel);
    }
}

void CraftPanelUI::toggleCraftPanel(bool show) {
    if (show) {
        buildCraftPanel();
        _recipes.clear();
        _pageIndex = 0;
        refreshCraftPanel();
        if (_panelNode) _panelNode->setVisible(true);
    } else {
        if (_panelNode) _panelNode->setVisible(false);
    }
}

bool CraftPanelUI::isVisible() const {
    return _panelNode && _panelNode->isVisible();
}

void CraftPanelUI::setCategory(Game::RecipeCategory cat) {
    if (_category == cat) return;
    _category = cat;
    _recipes.clear();
    _pageIndex = 0;
    updateTitle();
    updateTabsVisual();
    refreshCraftPanel();
}

void CraftPanelUI::rebuildRecipes() {
    Game::CategoryRecipeFilter filter(_category);
    _recipes = Game::RecipeBook::filtered(filter);
}

void CraftPanelUI::updateTitle() {
    if (!_titleLabel) return;
    const char* cat = "Placeable";
    switch (_category) {
        case Game::RecipeCategory::Placeable: cat = "Placeable"; break;
        case Game::RecipeCategory::Food: cat = "Food"; break;
        default: cat = "Placeable"; break;
    }
    _titleLabel->setString(StringUtils::format("Crafting - %s", cat));
}

void CraftPanelUI::updateTabsVisual() {
    auto setActive = [this](Label* tab, Game::RecipeCategory cat) {
        if (!tab) return;
        bool active = (_category == cat);
        tab->setColor(active ? Color3B::YELLOW : Color3B::WHITE);
    };
    setActive(_tabPlaceable, Game::RecipeCategory::Placeable);
    setActive(_tabFood, Game::RecipeCategory::Food);
}

std::string CraftPanelUI::formatIngredientsLine(const Game::RecipeBase& recipe) const {
    std::string s;
    const auto& req = recipe.ingredients();
    for (std::size_t i = 0; i < req.size(); ++i) {
        const auto& it = req[i];
        int have = _inventory ? _inventory->countItems(it.type) : 0;
        if (!s.empty()) {
            s += " + ";
        }
        s += StringUtils::format("%s x%d (%d)", Game::itemName(it.type), it.qty, have);
    }
    return s;
}

}
