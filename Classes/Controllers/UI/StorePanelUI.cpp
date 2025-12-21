#include "Controllers/UI/StorePanelUI.h"
#include "ui/CocosGUI.h"
#include "Game/Crops/crop/CropBase.h"

using namespace cocos2d;

namespace Controllers {

namespace {
    const float STORE_UI_SCALE = 1.3f;
}

// 构建商店面板节点：背景、标题、关闭按钮与翻页按钮
void StorePanelUI::buildStorePanel() {
    if (_panelNode) return;
    _panelNode = Node::create();
    if (_scene) _scene->addChild(_panelNode, 4);
    _panelNode->setVisible(false);
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _panelNode->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));

    auto bg = DrawNode::create();
    float w = 480.f * STORE_UI_SCALE, h = 300.f * STORE_UI_SCALE;
    Vec2 v[4] = { Vec2(-w/2,-h/2), Vec2(w/2,-h/2), Vec2(w/2,h/2), Vec2(-w/2,h/2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f,0.f,0.f,0.85f));
    _panelNode->addChild(bg);

    _titleLabel = Label::createWithTTF("", "fonts/arial.ttf", 24 * STORE_UI_SCALE);
    _titleLabel->setPosition(Vec2(0, h/2 - 26 * STORE_UI_SCALE));
    _panelNode->addChild(_titleLabel);
    updateTitle();

    auto closeBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(18 * STORE_UI_SCALE);
    closeBtn->setScale9Enabled(true);
    closeBtn->setContentSize(Size(36 * STORE_UI_SCALE, 36 * STORE_UI_SCALE));
    closeBtn->setPosition(Vec2(w/2 - 20 * STORE_UI_SCALE, h/2 - 20 * STORE_UI_SCALE));
    closeBtn->addClickEventListener([this](Ref*){ toggleStorePanel(false); });
    _panelNode->addChild(closeBtn);

    _tabsNode = Node::create();
    _panelNode->addChild(_tabsNode);
    float tabY = h/2 + 20 * STORE_UI_SCALE;
    _produceTab = Label::createWithTTF("Produce", "fonts/arial.ttf", 20 * STORE_UI_SCALE);
    _produceTab->setPosition(Vec2(-80 * STORE_UI_SCALE, tabY));
    _tabsNode->addChild(_produceTab);
    _mineralTab = Label::createWithTTF("Minerals", "fonts/arial.ttf", 20 * STORE_UI_SCALE);
    _mineralTab->setPosition(Vec2(80 * STORE_UI_SCALE, tabY));
    _tabsNode->addChild(_mineralTab);

    auto produceListener = EventListenerTouchOneByOne::create();
    produceListener->setSwallowTouches(true);
    produceListener->onTouchBegan = [this](Touch* t, Event* e){
        auto target = static_cast<Label*>(e->getCurrentTarget());
        Vec2 p = target->convertToNodeSpace(t->getLocation());
        Size s = target->getContentSize();
        Rect r(0, 0, s.width, s.height);
        if (r.containsPoint(p)) { target->setScale(0.9f); return true; }
        return false;
    };
    produceListener->onTouchEnded = [this](Touch* t, Event* e){
        if (_produceTab) _produceTab->setScale(1.0f);
        setCategory(StoreCategory::Produce);
    };
    _tabsNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(produceListener, _produceTab);

    auto mineralListener = EventListenerTouchOneByOne::create();
    mineralListener->setSwallowTouches(true);
    mineralListener->onTouchBegan = [this](Touch* t, Event* e){
        auto target = static_cast<Label*>(e->getCurrentTarget());
        Vec2 p = target->convertToNodeSpace(t->getLocation());
        Size s = target->getContentSize();
        Rect r(0, 0, s.width, s.height);
        if (r.containsPoint(p)) { target->setScale(0.9f); return true; }
        return false;
    };
    mineralListener->onTouchEnded = [this](Touch* t, Event* e){
        if (_mineralTab) _mineralTab->setScale(1.0f);
        setCategory(StoreCategory::Mineral);
    };
    _tabsNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mineralListener, _mineralTab);
    updateTabsVisual();

    _listNode = Node::create();
    _panelNode->addChild(_listNode);

    _storeController = std::make_unique<StoreController>(_inventory);

    // 翻页按钮
    auto prevBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    prevBtn->setTitleText("<"); prevBtn->setTitleFontSize(18 * STORE_UI_SCALE);
    prevBtn->setScale9Enabled(true); prevBtn->setContentSize(Size(36 * STORE_UI_SCALE, 36 * STORE_UI_SCALE));
    prevBtn->setPosition(Vec2(-w/2 + 30 * STORE_UI_SCALE, -h/2 + 30 * STORE_UI_SCALE));
    prevBtn->addClickEventListener([this](Ref*){
        if (_pageIndex > 0) { _pageIndex--; }
        refreshStorePanel();
    });
    _panelNode->addChild(prevBtn);

    auto nextBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    nextBtn->setTitleText(">"); nextBtn->setTitleFontSize(18 * STORE_UI_SCALE);
    nextBtn->setScale9Enabled(true); nextBtn->setContentSize(Size(36 * STORE_UI_SCALE, 36 * STORE_UI_SCALE));
    nextBtn->setPosition(Vec2(w/2 - 30 * STORE_UI_SCALE, -h/2 + 30 * STORE_UI_SCALE));
    nextBtn->addClickEventListener([this](Ref*){
        // 先刷新一次以确保 _items 准备好
        if (_items.empty()) refreshStorePanel();
        int total = static_cast<int>(_items.size());
        int maxPage = std::max(1, (total + _pageSize - 1)/_pageSize);
        if (_pageIndex < maxPage - 1) { _pageIndex++; }
        refreshStorePanel();
    });
    _panelNode->addChild(nextBtn);
}

// 刷新商店物品列表：根据分页在面板中绘制图标、名称、价格与购买/出售操作
void StorePanelUI::refreshStorePanel() {
    if (!_listNode || !_storeController) return;
    _listNode->removeAllChildren();
    if (_items.empty()) {
        rebuildItems();
    }

    float startY = 80.0f * STORE_UI_SCALE;
    float gapY = 40.0f * STORE_UI_SCALE;
    int total = static_cast<int>(_items.size());
    int startIdx = std::max(0, _pageIndex * _pageSize);
    int endIdx = std::min(total, startIdx + _pageSize);
    if (startIdx >= total) { _pageIndex = std::max(0, (total - 1) / _pageSize); startIdx = _pageIndex * _pageSize; endIdx = std::min(total, startIdx + _pageSize); }

    auto pageLabel = Label::createWithTTF(StringUtils::format("Page %d/%d", _pageIndex + 1, std::max(1, (total + _pageSize - 1)/_pageSize)), "fonts/arial.ttf", 16 * STORE_UI_SCALE);
    pageLabel->setPosition(Vec2(0, -120 * STORE_UI_SCALE));
    _listNode->addChild(pageLabel);

    for (int row = 0, i = startIdx; i < endIdx; ++i, ++row) {
        auto type = _items[i];
        float y = startY - row * gapY;
        std::string iconPath = Game::itemIconPath(type);
        if (iconPath.empty()) {
            switch (type) {
                case Game::ItemType::Coal:         iconPath = "Mineral/Coal.png"; break;
                case Game::ItemType::CopperGrain: iconPath = "Mineral/copperGrain.png"; break;
                case Game::ItemType::CopperIngot: iconPath = "Mineral/copperIngot.png"; break;
                case Game::ItemType::IronGrain:   iconPath = "Mineral/ironGrain.png"; break;
                case Game::ItemType::IronIngot:   iconPath = "Mineral/ironIngot.png"; break;
                case Game::ItemType::GoldGrain:   iconPath = "Mineral/goldGrain.png"; break;
                case Game::ItemType::GoldIngot:   iconPath = "Mineral/goldIngot.png"; break;
                default: break;
            }
        }
        if (!iconPath.empty()) {
            auto icon = Sprite::create();
            icon->setTexture(iconPath);
            if (icon->getTexture()) {
                float targetH = 24.0f * STORE_UI_SCALE; float targetW = 24.0f * STORE_UI_SCALE;
                auto cs = icon->getContentSize();
                float sx = (cs.width > 0) ? (targetW / cs.width) : 1.0f;
                float sy = (cs.height > 0) ? (targetH / cs.height) : 1.0f;
                icon->setScale(std::min(sx, sy));
                icon->setPosition(Vec2(-200 * STORE_UI_SCALE, y));
                _listNode->addChild(icon);
            }
        }
        auto nameLabel = Label::createWithTTF(Game::itemName(type), "fonts/arial.ttf", 20 * STORE_UI_SCALE);
        nameLabel->setAnchorPoint(Vec2(0, 0.5f));
        nameLabel->setPosition(Vec2(-180 * STORE_UI_SCALE, y));
        _listNode->addChild(nameLabel);
        bool isSeed = Game::isSeed(type);
        bool isFishItem = Game::isFish(type);
        int price = isSeed ? _storeController->getSeedPrice(type) : _storeController->getItemPrice(type);
        auto priceLabel = Label::createWithTTF(StringUtils::format("%d G", price), "fonts/arial.ttf", 20 * STORE_UI_SCALE);
        priceLabel->setAnchorPoint(Vec2(1, 0.5f));
        priceLabel->setPosition(Vec2(80 * STORE_UI_SCALE, y));
        priceLabel->setColor(Color3B::YELLOW);
        _listNode->addChild(priceLabel);
        if (!isFishItem) {
            auto buyLabel = Label::createWithTTF("[Buy]", "fonts/arial.ttf", 20 * STORE_UI_SCALE);
            buyLabel->setPosition(Vec2(120 * STORE_UI_SCALE, y));
            buyLabel->setColor(Color3B::GREEN);
            auto buyListener = EventListenerTouchOneByOne::create();
            buyListener->setSwallowTouches(true);
            buyListener->onTouchBegan = [buyLabel](Touch* t, Event* e){
                auto target = static_cast<Label*>(e->getCurrentTarget());
                Vec2 p = target->convertToNodeSpace(t->getLocation());
                Size s = target->getContentSize();
                Rect r(0, 0, s.width, s.height);
                if (r.containsPoint(p)) { target->setScale(0.9f); return true; }
                return false;
            };
            buyListener->onTouchEnded = [this, type, isSeed, buyLabel](Touch* t, Event* e){
                buyLabel->setScale(1.0f);
                bool ok = false;
                if (_storeController) {
                    ok = isSeed ? _storeController->buySeed(type) : _storeController->buyItem(type);
                }
                if (onPurchased) onPurchased(ok);
            };
            _listNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(buyListener, buyLabel);
            _listNode->addChild(buyLabel);
        }

        bool canSell = !isSeed && Game::itemPrice(type) > 0;
        if (canSell) {
            auto sellLabel = Label::createWithTTF("[Sell]", "fonts/arial.ttf", 20 * STORE_UI_SCALE);
            sellLabel->setPosition(Vec2(200 * STORE_UI_SCALE, y));
            sellLabel->setColor(Color3B::RED);
            auto sellListener = EventListenerTouchOneByOne::create();
            sellListener->setSwallowTouches(true);
            sellListener->onTouchBegan = [sellLabel](Touch* t, Event* e){
                auto target = static_cast<Label*>(e->getCurrentTarget());
                Vec2 p = target->convertToNodeSpace(t->getLocation());
                Size s = target->getContentSize();
                Rect r(0, 0, s.width, s.height);
                if (r.containsPoint(p)) { target->setScale(0.9f); return true; }
                return false;
            };
            sellListener->onTouchEnded = [this, type, sellLabel](Touch* t, Event* e){
                sellLabel->setScale(1.0f);
                bool ok = false;
                if (_storeController && _inventory) {
                    int have = _inventory->countItems(type);
                    if (have > 0) {
                        ok = _storeController->sellItem(type, have);
                    }
                }
                if (onPurchased) onPurchased(ok);
            };
            _listNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(sellListener, sellLabel);
            _listNode->addChild(sellLabel);
        }
    }
}

void StorePanelUI::toggleStorePanel(bool show) {
    if (show) {
        buildStorePanel();
        _items.clear();
        _pageIndex = 0;
        refreshStorePanel();
        if (_panelNode) _panelNode->setVisible(true);
    } else {
        if (_panelNode) _panelNode->setVisible(false);
    }
}

void StorePanelUI::setCategory(StoreCategory cat) {
    if (_category == cat) return;
    _category = cat;
    _items.clear();
    _pageIndex = 0;
    updateTitle();
    updateTabsVisual();
    refreshStorePanel();
}

bool StorePanelUI::isVisible() const {
    return _panelNode && _panelNode->isVisible();
}

void StorePanelUI::rebuildItems() {
    _items.clear();
    std::vector<Game::ItemType> seeds = {
        Game::ItemType::ParsnipSeed,
        Game::ItemType::BlueberrySeed,
        Game::ItemType::EggplantSeed,
        Game::ItemType::CornSeed,
        Game::ItemType::StrawberrySeed
    };
    std::vector<Game::ItemType> produce = {
        Game::ItemType::Parsnip,
        Game::ItemType::Blueberry,
        Game::ItemType::Eggplant,
        Game::ItemType::Corn,
        Game::ItemType::Strawberry,
        Game::ItemType::Egg,
        Game::ItemType::Milk,
        Game::ItemType::Wool
    };
    std::vector<Game::ItemType> fish = {
        Game::ItemType::Carp,
        Game::ItemType::BreamFish,
        Game::ItemType::Sardine,
        Game::ItemType::Salmon,
        Game::ItemType::RainbowTrout,
        Game::ItemType::MidnightCarp,
        Game::ItemType::LargemouthBass,
        Game::ItemType::Sturgeon,
        Game::ItemType::SmallmouthBass,
        Game::ItemType::Tilapia,
        Game::ItemType::Tuna,
        Game::ItemType::Globefish,
        Game::ItemType::Anchovy,
        Game::ItemType::BlueDiscus,
        Game::ItemType::Clam,
        Game::ItemType::Crab,
        Game::ItemType::Lobster,
        Game::ItemType::Shrimp
    };
    std::vector<Game::ItemType> minerals = {
        Game::ItemType::Coal,
        Game::ItemType::CopperGrain,
        Game::ItemType::CopperIngot,
        Game::ItemType::IronGrain,
        Game::ItemType::IronIngot,
        Game::ItemType::GoldGrain,
        Game::ItemType::GoldIngot
    };
    if (_category == StoreCategory::Produce) {
        _items.reserve(seeds.size() + produce.size() + fish.size());
        _items.insert(_items.end(), seeds.begin(), seeds.end());
        _items.insert(_items.end(), produce.begin(), produce.end());
        _items.insert(_items.end(), fish.begin(), fish.end());
    } else {
        _items = minerals;
    }
}

void StorePanelUI::updateTitle() {
    if (!_titleLabel) return;
    const char* text = (_category == StoreCategory::Produce) ? "Produce Store" : "Mineral Store";
    _titleLabel->setString(text);
}

void StorePanelUI::updateTabsVisual() {
    Color3B selected(255, 215, 0);
    Color3B normal(200, 200, 200);
    if (_produceTab) {
        _produceTab->setColor(_category == StoreCategory::Produce ? selected : normal);
    }
    if (_mineralTab) {
        _mineralTab->setColor(_category == StoreCategory::Mineral ? selected : normal);
    }
}

void AnimalStorePanelUI::buildAnimalStorePanel() {
    if (_panelNode) return;
    _panelNode = Node::create();
    if (_scene) _scene->addChild(_panelNode, 4);
    _panelNode->setVisible(false);
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _panelNode->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));

    auto bg = DrawNode::create();
    float w = 420.f * STORE_UI_SCALE;
    float h = 220.f * STORE_UI_SCALE;
    Vec2 v[4] = { Vec2(-w/2,-h/2), Vec2(w/2,-h/2), Vec2(w/2,h/2), Vec2(-w/2,h/2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f,0.f,0.f,0.85f));
    _panelNode->addChild(bg);

    auto title = Label::createWithTTF("Animal Store", "fonts/arial.ttf", 24 * STORE_UI_SCALE);
    title->setPosition(Vec2(0, h/2 - 26 * STORE_UI_SCALE));
    _panelNode->addChild(title);

    auto closeBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(18 * STORE_UI_SCALE);
    closeBtn->setScale9Enabled(true);
    closeBtn->setContentSize(Size(36 * STORE_UI_SCALE, 36 * STORE_UI_SCALE));
    closeBtn->setPosition(Vec2(w/2 - 20 * STORE_UI_SCALE, h/2 - 20 * STORE_UI_SCALE));
    closeBtn->addClickEventListener([this](Ref*){ toggleAnimalStorePanel(false); });
    _panelNode->addChild(closeBtn);

    _listNode = Node::create();
    _panelNode->addChild(_listNode);
}

void AnimalStorePanelUI::refreshAnimalStorePanel() {
    if (!_listNode) return;
    _listNode->removeAllChildren();
    std::vector<Game::AnimalType> animals = {
        Game::AnimalType::Chicken,
        Game::AnimalType::Cow,
        Game::AnimalType::Sheep
    };
    float startY = 40.0f * STORE_UI_SCALE;
    float gapY = 40.0f * STORE_UI_SCALE;
    for (int row = 0; row < static_cast<int>(animals.size()); ++row) {
        auto type = animals[row];
        float y = startY - row * gapY;
        std::string iconPath;
        std::string name;
        switch (type) {
            case Game::AnimalType::Chicken:
                iconPath = "animal/chicken.png";
                name = "Chicken";
                break;
            case Game::AnimalType::Cow:
                iconPath = "animal/cow.png";
                name = "Cow";
                break;
            case Game::AnimalType::Sheep:
                iconPath = "animal/sheep.png";
                name = "Sheep";
                break;
        }
        if (!iconPath.empty()) {
            auto icon = Sprite::create();
            icon->setTexture(iconPath);
            if (icon->getTexture()) {
                float targetH = 32.0f * STORE_UI_SCALE;
                float targetW = 32.0f * STORE_UI_SCALE;
                auto cs = icon->getContentSize();
                float sx = (cs.width > 0) ? (targetW / cs.width) : 1.0f;
                float sy = (cs.height > 0) ? (targetH / cs.height) : 1.0f;
                icon->setScale(std::min(sx, sy));
                icon->setPosition(Vec2(-140 * STORE_UI_SCALE, y));
                _listNode->addChild(icon);
            }
        }
        auto nameLabel = Label::createWithTTF(name, "fonts/arial.ttf", 20 * STORE_UI_SCALE);
        nameLabel->setAnchorPoint(Vec2(0, 0.5f));
        nameLabel->setPosition(Vec2(-110 * STORE_UI_SCALE, y));
        _listNode->addChild(nameLabel);

        long long price = Game::animalPrice(type);
        auto priceLabel = Label::createWithTTF(StringUtils::format("%lld G", price), "fonts/arial.ttf", 20 * STORE_UI_SCALE);
        priceLabel->setAnchorPoint(Vec2(1, 0.5f));
        priceLabel->setPosition(Vec2(60 * STORE_UI_SCALE, y));
        priceLabel->setColor(Color3B::YELLOW);
        _listNode->addChild(priceLabel);

        auto buyLabel = Label::createWithTTF("[Buy]", "fonts/arial.ttf", 20 * STORE_UI_SCALE);
        buyLabel->setPosition(Vec2(120 * STORE_UI_SCALE, y));
        buyLabel->setColor(Color3B::GREEN);
        auto buyListener = EventListenerTouchOneByOne::create();
        buyListener->setSwallowTouches(true);
        buyListener->onTouchBegan = [buyLabel](Touch* t, Event* e){
            auto target = static_cast<Label*>(e->getCurrentTarget());
            Vec2 p = target->convertToNodeSpace(t->getLocation());
            Size s = target->getContentSize();
            Rect r(0, 0, s.width, s.height);
            if (r.containsPoint(p)) { target->setScale(0.9f); return true; }
            return false;
        };
        buyListener->onTouchEnded = [this, type, buyLabel](Touch* t, Event* e){
            buyLabel->setScale(1.0f);
            bool ok = false;
            if (onBuyAnimal) {
                ok = onBuyAnimal(type);
            }
            if (onPurchased) {
                onPurchased(ok);
            }
        };
        _listNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(buyListener, buyLabel);
        _listNode->addChild(buyLabel);
    }
}

void AnimalStorePanelUI::toggleAnimalStorePanel(bool show) {
    if (show) {
        buildAnimalStorePanel();
        refreshAnimalStorePanel();
        if (_panelNode) _panelNode->setVisible(true);
    } else {
        if (_panelNode) _panelNode->setVisible(false);
    }
}

bool AnimalStorePanelUI::isVisible() const {
    return _panelNode && _panelNode->isVisible();
}

}
