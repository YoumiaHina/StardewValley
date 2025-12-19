#include "CustomizationScene.h"
#include "Scenes/RoomScene.h"
#include "ui/CocosGUI.h"
#include "Game/WorldState.h"

USING_NS_CC;

Scene* CustomizationScene::createScene() {
    return CustomizationScene::create();
}

bool CustomizationScene::init() {
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // Background
    auto bg = LayerColor::create(Color4B(50, 50, 50, 255));
    this->addChild(bg);

    // Title
    auto label = Label::createWithTTF("Customize Character", "fonts/Marker Felt.ttf", 24);
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));
    this->addChild(label, 1);

    // Character Node
    _character = Game::PlayerView::create();
    _character->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 50));
    _character->setScale(4.0f); // Scale up
    _character->setMoving(true);
    _character->setDirection(Game::IPlayerView::Direction::DOWN);
    this->addChild(_character);

    auto& ws = Game::globalState();
    _currentShirt = ws.playerShirt;
    _currentPants = ws.playerPants;
    _currentHair = ws.playerHair;
    int r = ws.playerHairR;
    int g = ws.playerHairG;
    int b = ws.playerHairB;

    int maxShirt = Game::PlayerView::getMaxShirtStyles();
    int maxPants = Game::PlayerView::getMaxPantsStyles();
    int maxHair = Game::PlayerView::getMaxHairStyles();
    if (_currentShirt < 0 || _currentShirt >= maxShirt) _currentShirt = 0;
    if (_currentPants < 0 || _currentPants >= maxPants) _currentPants = 0;
    if (_currentHair < 0 || _currentHair >= maxHair) _currentHair = 0;

    _character->setShirtStyle(_currentShirt);
    _character->setPantsStyle(_currentPants);
    _character->setHairStyle(_currentHair);
    _character->setHairColor(Color3B(r, g, b));

    // UI Controls
    float startY = visibleSize.height / 2 - 50;
    float gapY = 40;

    auto createControl = [&](const std::string& name, int& valRef, float y, std::function<void(int)> callback) {
        auto labelName = Label::createWithTTF(name, "fonts/arial.ttf", 18);
        labelName->setPosition(Vec2(visibleSize.width/2 - 100, y));
        this->addChild(labelName);

        auto btnPrev = MenuItemLabel::create(Label::createWithTTF("<", "fonts/arial.ttf", 24), 
            [callback](Ref*){ callback(-1); });
        btnPrev->setPosition(Vec2(visibleSize.width/2 - 20, y));

        auto btnNext = MenuItemLabel::create(Label::createWithTTF(">", "fonts/arial.ttf", 24), 
            [callback](Ref*){ callback(1); });
        btnNext->setPosition(Vec2(visibleSize.width/2 + 20, y));
        
        auto menu = Menu::create(btnPrev, btnNext, nullptr);
        menu->setPosition(Vec2::ZERO);
        this->addChild(menu);
        
        return Label::createWithTTF("0", "fonts/arial.ttf", 18);
    };

    _shirtLabel = createControl("Shirt", _currentShirt, startY, [this](int d){ changeShirt(d); });
    _shirtLabel->setPosition(Vec2(visibleSize.width/2, startY));
    this->addChild(_shirtLabel);

    _hairLabel = createControl("Hair", _currentHair, startY - gapY*2, [this](int d){ changeHair(d); });
    _hairLabel->setPosition(Vec2(visibleSize.width/2, startY - gapY*2));
    this->addChild(_hairLabel);

    // Color Buttons
    float colorY = startY - gapY*3;
    std::vector<Color3B> colors = {
        Color3B::WHITE, Color3B::BLACK, Color3B::RED, Color3B::GREEN, Color3B::BLUE, 
        Color3B(255, 255, 0), Color3B(0, 255, 255), Color3B(255, 0, 255),
        Color3B(139, 69, 19), // Brown
        Color3B(255, 165, 0) // Orange
    };

    Vector<MenuItem*> colorItems;
    for (int i = 0; i < colors.size(); i++) {
        auto sprite = Sprite::create();
        sprite->setTextureRect(Rect(0, 0, 20, 20));
        sprite->setColor(colors[i]);
        auto item = MenuItemSprite::create(sprite, sprite, [this, c=colors[i]](Ref*){
            changeHairColor(c);
        });
        item->setPosition(Vec2(visibleSize.width/2 - 100 + (i * 25), colorY));
        colorItems.pushBack(item);
    }
    auto colorMenu = Menu::createWithArray(colorItems);
    colorMenu->setPosition(Vec2::ZERO);
    this->addChild(colorMenu);

    // Start Button
    auto startLabel = Label::createWithTTF("START GAME", "fonts/Marker Felt.ttf", 32);
    auto startItem = MenuItemLabel::create(startLabel, CC_CALLBACK_1(CustomizationScene::onStartGame, this));
    startItem->setPosition(Vec2(visibleSize.width/2, 50));
    
    auto startMenu = Menu::create(startItem, nullptr);
    startMenu->setPosition(Vec2::ZERO);
    this->addChild(startMenu);

    // Schedule Update for animation
    this->schedule([this](float dt){
        _character->updateAnimation(dt);
    }, "anim_key");

    updateLabels();

    return true;
}

void CustomizationScene::changeShirt(int delta) {
    _currentShirt += delta;
    int max = Game::PlayerView::getMaxShirtStyles();
    if (_currentShirt < 0) _currentShirt = max - 1;
    if (_currentShirt >= max) _currentShirt = 0;
    
    _character->setShirtStyle(_currentShirt);
    updateLabels();
}

void CustomizationScene::changePants(int delta) {
    _currentPants += delta;
    int max = Game::PlayerView::getMaxPantsStyles();
    if (_currentPants < 0) _currentPants = max - 1;
    if (_currentPants >= max) _currentPants = 0;
    
    _character->setPantsStyle(_currentPants);
    updateLabels();
}

void CustomizationScene::changeHair(int delta) {
    _currentHair += delta;
    int max = Game::PlayerView::getMaxHairStyles();
    if (_currentHair < 0) _currentHair = max - 1;
    if (_currentHair >= max) _currentHair = 0;
    
    _character->setHairStyle(_currentHair);
    updateLabels();
}

void CustomizationScene::changeHairColor(const cocos2d::Color3B& color) {
    _character->setHairColor(color);
}

void CustomizationScene::updateLabels() {
    _shirtLabel->setString(std::to_string(_currentShirt + 1));
    if (_pantsLabel) _pantsLabel->setString(std::to_string(_currentPants));
    _hairLabel->setString(std::to_string(_currentHair + 1));
}

void CustomizationScene::onStartGame(Ref* sender) {
    auto& ws = Game::globalState();
    ws.playerShirt = _character->getShirtStyle();
    ws.playerPants = _character->getPantsStyle();
    ws.playerHair = _character->getHairStyle();
    auto color = _character->getHairColor();
    ws.playerHairR = color.r;
    ws.playerHairG = color.g;
    ws.playerHairB = color.b;

    // Transition
    Director::getInstance()->replaceScene(TransitionFade::create(1.0, RoomScene::createScene()));
}
