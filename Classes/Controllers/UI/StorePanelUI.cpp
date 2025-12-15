#include "Controllers/UI/StorePanelUI.h"
#include "ui/CocosGUI.h"
#include "Game/Crop.h"

using namespace cocos2d;

namespace Controllers {

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
    float w = 400.f, h = 300.f;
    Vec2 v[4] = { Vec2(-w/2,-h/2), Vec2(w/2,-h/2), Vec2(w/2,h/2), Vec2(-w/2,h/2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f,0.f,0.f,0.85f));
    _panelNode->addChild(bg);

    auto title = Label::createWithTTF("General Store", "fonts/arial.ttf", 24);
    title->setPosition(Vec2(0, h/2 - 26));
    _panelNode->addChild(title);

    auto closeBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(18);
    closeBtn->setScale9Enabled(true);
    closeBtn->setContentSize(Size(36, 36));
    closeBtn->setPosition(Vec2(w/2 - 20, h/2 - 20));
    closeBtn->addClickEventListener([this](Ref*){ toggleStorePanel(false); });
    _panelNode->addChild(closeBtn);

    _listNode = Node::create();
    _panelNode->addChild(_listNode);

    _storeController = std::make_unique<StoreController>(_inventory);

    // 翻页按钮
    auto prevBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    prevBtn->setTitleText("<"); prevBtn->setTitleFontSize(18);
    prevBtn->setScale9Enabled(true); prevBtn->setContentSize(Size(36, 36));
    prevBtn->setPosition(Vec2(-w/2 + 30, -h/2 + 30));
    prevBtn->addClickEventListener([this](Ref*){
        if (_pageIndex > 0) { _pageIndex--; }
        refreshStorePanel();
    });
    _panelNode->addChild(prevBtn);

    auto nextBtn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    nextBtn->setTitleText(">"); nextBtn->setTitleFontSize(18);
    nextBtn->setScale9Enabled(true); nextBtn->setContentSize(Size(36, 36));
    nextBtn->setPosition(Vec2(w/2 - 30, -h/2 + 30));
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
        std::vector<Game::ItemType> minerals = {
            Game::ItemType::Coal,
            Game::ItemType::CopperGrain,
            Game::ItemType::CopperIngot,
            Game::ItemType::IronGrain,
            Game::ItemType::IronIngot,
            Game::ItemType::GoldGrain,
            Game::ItemType::GoldIngot
        };
        _items.reserve(seeds.size() + produce.size() + minerals.size());
        _items.insert(_items.end(), seeds.begin(), seeds.end());
        _items.insert(_items.end(), produce.begin(), produce.end());
        _items.insert(_items.end(), minerals.begin(), minerals.end());
    }

    float startY = 80.0f;
    float gapY = 40.0f;
    int total = static_cast<int>(_items.size());
    int startIdx = std::max(0, _pageIndex * _pageSize);
    int endIdx = std::min(total, startIdx + _pageSize);
    if (startIdx >= total) { _pageIndex = std::max(0, (total - 1) / _pageSize); startIdx = _pageIndex * _pageSize; endIdx = std::min(total, startIdx + _pageSize); }

    auto pageLabel = Label::createWithTTF(StringUtils::format("Page %d/%d", _pageIndex + 1, std::max(1, (total + _pageSize - 1)/_pageSize)), "fonts/arial.ttf", 16);
    pageLabel->setPosition(Vec2(0, -120));
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
                float targetH = 24.0f; float targetW = 24.0f;
                auto cs = icon->getContentSize();
                float sx = (cs.width > 0) ? (targetW / cs.width) : 1.0f;
                float sy = (cs.height > 0) ? (targetH / cs.height) : 1.0f;
                icon->setScale(std::min(sx, sy));
                icon->setPosition(Vec2(-200, y));
                _listNode->addChild(icon);
            }
        }
        auto nameLabel = Label::createWithTTF(Game::itemName(type), "fonts/arial.ttf", 20);
        nameLabel->setAnchorPoint(Vec2(0, 0.5f));
        nameLabel->setPosition(Vec2(-180, y));
        _listNode->addChild(nameLabel);
        bool isSeed = Game::isSeed(type);
        int price = isSeed ? _storeController->getSeedPrice(type) : _storeController->getItemPrice(type);
        auto priceLabel = Label::createWithTTF(StringUtils::format("%d G", price), "fonts/arial.ttf", 20);
        priceLabel->setAnchorPoint(Vec2(1, 0.5f));
        priceLabel->setPosition(Vec2(80, y));
        priceLabel->setColor(Color3B::YELLOW);
        _listNode->addChild(priceLabel);
        auto buyLabel = Label::createWithTTF("[Buy]", "fonts/arial.ttf", 20);
        buyLabel->setPosition(Vec2(120, y));
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

        bool canSell = !isSeed && Game::itemPrice(type) > 0;
        if (canSell) {
            auto sellLabel = Label::createWithTTF("[Sell]", "fonts/arial.ttf", 20);
            sellLabel->setPosition(Vec2(200, y));
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
        refreshStorePanel();
        if (_panelNode) _panelNode->setVisible(true);
    } else {
        if (_panelNode) _panelNode->setVisible(false);
    }
}

}
